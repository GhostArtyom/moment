/**
 * defining_map.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "symbol_table_map.h"

#include "map_core.h"

#include "scenarios/context.h"

#include "symbolic/symbol_table.h"
#include "utilities/dynamic_bitset.h"

#include <limits>
#include <sstream>

namespace Moment::Derived {

    namespace {
        constexpr bool is_close(const double x, const double y, const double eps_mult = 1.0) {
            return (std::abs(x - y) < std::numeric_limits<double>::epsilon() * std::max(std::abs(x), std::abs(y)));
        }

        std::pair<std::vector<symbol_name_t>, DynamicBitset<size_t>>
        unzip_indices(const SymbolTable& origin_symbols, size_t matrix_size) {
            auto output = std::make_pair(std::vector<symbol_name_t>(),  DynamicBitset<size_t>{matrix_size, false});
            output.first.reserve(matrix_size);

            for (size_t i = 0; i < matrix_size; ++i) {
                auto [symbol_id, conjugate] = origin_symbols.OSGIndex(i);
                output.first.emplace_back(symbol_id);
                if (conjugate) {
                    output.second.set(i);
                }
            }
            return output;
        }
    }


    SymbolTableMap::SymbolTableMap(const SymbolTable& origin, SymbolTable& target,
                                   std::unique_ptr<MapCore> core_in,
                                   std::unique_ptr<SolvedMapCore> solution_in)
        : origin_symbols{origin}, target_symbols{target},
            core{std::move(core_in)}, core_solution{std::move(solution_in)} {
        if (!this->core) {
            throw errors::bad_map{"Map cannot be constructed without a MapCore."};
        }
        if (!this->core_solution) {
            throw errors::bad_map{"Map cannot be constructed without a SolvedMapCore."};
        }

        auto [osg_to_sym, conjugates] = unzip_indices(this->origin_symbols, this->core->initial_size);
        this->construct_map(osg_to_sym);
    }

    SymbolTableMap::SymbolTableMap(const SymbolTable& origin, SymbolTable& target,
                                   const MapCoreProcessor& processor, const Eigen::MatrixXd& src)
        : origin_symbols{origin}, target_symbols{target} {

        auto [osg_to_sym, conjugates] = unzip_indices(this->origin_symbols, src.cols());

        this->core = std::make_unique<MapCore>(std::move(conjugates), src);
        this->core_solution = core->accept(processor);
        this->construct_map(osg_to_sym);
    }

    SymbolTableMap::SymbolTableMap(const SymbolTable& origin, SymbolTable& target,
                                   const MapCoreProcessor& processor, const Eigen::SparseMatrix<double>& src)
        : origin_symbols{origin}, target_symbols{target} {

        auto [osg_to_sym, conjugates] = unzip_indices(this->origin_symbols, src.cols());

        this->core = std::make_unique<MapCore>(std::move(conjugates), src);
        this->core_solution = core->accept(processor);
        this->construct_map(osg_to_sym);
    }


    void SymbolTableMap::construct_map(const std::vector<symbol_name_t>& osg_to_symbols) {
        assert(this->core);
        assert(this->core_solution);

        // Check core and solution match.
        this->core->check_solution(*this->core_solution);

        // First, build fixed constants.
        this->map.assign(origin_symbols.size(), SymbolCombo::Zero()); // 0 -> 0 always
        this->map[0] = SymbolCombo::Zero();      // 0 -> 0 always.
        this->map[1] = SymbolCombo::Scalar(1.0); // 1 -> 1 always.

        // Create scalars remap:
        for (auto [row_id, scalar] : this->core->constants) {
            auto [symbol_id, conjugated] = origin_symbols.OSGIndex(row_id);
            assert(!conjugated);
            this->map[symbol_id] = SymbolCombo::Scalar(scalar);
        }

        const auto& raw_map = this->core_solution->map;
        const auto& raw_inv_map = this->core_solution->inv_map;

        // Create non-trivial map:
        Eigen::Index core_col_id = 0;
        for (auto non_trivial_idx : this->core->nontrivial_cols) {

            const symbol_name_t source_symbol = osg_to_symbols[non_trivial_idx];

            SymbolCombo::storage_t from_x_to_y;

            // Constant offset, if any:
            if (this->core->core_offset[core_col_id] != 0.0) {
                from_x_to_y.emplace_back(static_cast<symbol_name_t>(1), this->core->core_offset[core_col_id]);
            }
            // Non-trivial parts
            for (Eigen::Index map_col_id = 0, map_col_max = raw_map.cols();
                map_col_id < map_col_max; ++map_col_id) {
                const double value = raw_map(core_col_id, map_col_id);
                if (abs(value) != 0.0) {
                    from_x_to_y.emplace_back(osg_to_symbols[map_col_id], value);
                }
            }
            // Create mapping
            this->map[source_symbol] = SymbolCombo{std::move(from_x_to_y)};
            ++core_col_id;
        }

        // Create reverse map:
        this->inverse_map.reserve(2 + this->core_solution->output_symbols);
        this->inverse_map.emplace_back(SymbolCombo::Zero());      // 0 -> 0 always.
        this->inverse_map.emplace_back(SymbolCombo::Scalar(1.0)); // 1 -> 1 always.

        for (Eigen::Index im_row_id = 0; im_row_id < this->core_solution->output_symbols; ++im_row_id) {
            SymbolCombo::storage_t from_y_to_x;
            for (Eigen::Index im_col_id = 0, im_col_max = this->core_solution->inv_map.cols();
                im_col_id < im_col_max; ++im_col_id) {
                const auto as_symbol = static_cast<symbol_name_t>(im_col_id + 2);
                const double value = raw_inv_map(im_row_id, im_col_id);
                if (abs(value) != 0.0) {
                    from_y_to_x.emplace_back(as_symbol, value);
                }
            }
            this->inverse_map.emplace_back(std::move(from_y_to_x));
        }
        assert(this->inverse_map.size() == this->core_solution->output_symbols+2);
    }

    size_t SymbolTableMap::populate_target_symbols() {
        if (2 != this->target_symbols.size()) {
            throw errors::bad_map{"Target SymbolTable should be empty (except for zero and identity)."};
        }

        return this->map.size();
    }



    SymbolTableMap::~SymbolTableMap() noexcept = default;


    const SymbolCombo& SymbolTableMap::operator()(symbol_name_t symbol_id) const {
        // Bounds check
        if ((symbol_id < 0) || (symbol_id >= this->map.size())) {
            std::stringstream ss;
            ss << "Symbol " << symbol_id << " not defined in implied map.";
            throw errors::bad_map{ss.str()};
        }

        // Remap
        return this->map[symbol_id];
    }

    SymbolCombo SymbolTableMap::operator()(const SymbolExpression &symbol) const {
        // Get raw combo, or throw range error
        SymbolCombo output = (*this)(symbol.id);

        // Apply transformations (using target symbol table)
        output *= symbol.factor;
        if (symbol.conjugated) {
            output.conjugate_in_place(this->target_symbols);
        }

        return output;
    }

    const SymbolCombo& SymbolTableMap::inverse(symbol_name_t symbol_id) const {
        // Bounds check
        if ((symbol_id < 0) || (symbol_id >= this->inverse_map.size())) {
            std::stringstream ss;
            ss << "Symbol " << symbol_id << " not defined in inverse map.";
            throw errors::bad_map{ss.str()};
        }

        // Remap
        return this->inverse_map[symbol_id];
    }

    SymbolCombo SymbolTableMap::inverse(const SymbolExpression& symbol) const {
        // Get raw combo, or throw range error
        SymbolCombo output = this->inverse(symbol.id);

        // Apply transformations (using target symbol table)
        output *= symbol.factor;
        if (symbol.conjugated) {
            output.conjugate_in_place(this->target_symbols);
        }

        return output;
    }
}