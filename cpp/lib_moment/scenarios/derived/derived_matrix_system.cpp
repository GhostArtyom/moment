/**
 * derived_matrix_system.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "derived_matrix_system.h"

#include "derived_context.h"
#include "symbol_table_map.h"

#include "matrix/moment_matrix.h"
#include "matrix/localizing_matrix.h"

#include <cassert>

namespace Moment::Derived {
    DerivedMatrixSystem::DerivedMatrixSystem(std::shared_ptr<MatrixSystem>&& base_system, STMFactory&& stm_factory)
        : MatrixSystem(DerivedMatrixSystem::make_derived_context(*base_system)),
          base_ms_ptr{std::move(base_system)}
    {
        // Avoid deadlock. Should never occur...!
        assert(this->base_ms_ptr.get() != this);

        // Make map from factory (i.e. virtual call).
        this->map_ptr = stm_factory(this->base_ms_ptr->Symbols(), this->Symbols());

        // Make map core ?
        //this->map_ptr = std::make_unique<SymbolTableMap>(this->base_ms_ptr->Symbols(), this->Symbols(),
        //                                                 nullptr, nullptr);

        // Ensure source scenario defines a sufficiently large symbol table
        //this->base_system().generate_dictionary(2*level);

    }

    DerivedMatrixSystem::~DerivedMatrixSystem() noexcept = default;

    std::unique_ptr<Context> DerivedMatrixSystem::make_derived_context(const MatrixSystem& source) {
        return std::make_unique<Derived::DerivedContext>(source.Context());
    }

    std::unique_ptr<class MomentMatrix> DerivedMatrixSystem::createNewMomentMatrix(size_t level) {
        // First check if map is capable of defining this MM.


        // Check source moment matrix exists, create it if it doesn't
        const auto& source_matrix = [&]() -> const class MomentMatrix& {
            auto read_source_lock = this->base_system().get_read_lock();
            // Get, if exists
            auto index = this->base_system().find_moment_matrix(level);
            if (index >= 0) {
                return dynamic_cast<const class MomentMatrix&>(this->base_system()[index]);
            }
            read_source_lock.unlock();

            // Wait for write lock...
            auto write_source_lock = this->base_system().get_write_lock();
            auto [mm_index, mm] = this->base_system().create_moment_matrix(level);

            return mm; // write_source_lock unlocks
        }();


        throw std::runtime_error{"Not implemented."};
    }

    std::unique_ptr<class LocalizingMatrix>
    DerivedMatrixSystem::createNewLocalizingMatrix(const LocalizingMatrixIndex &lmi) {
        throw std::runtime_error{"Not implemented."};
    }

    const Derived::SymbolTableMap &DerivedMatrixSystem::map() const {
        if (!this->map_ptr) {
            throw errors::missing_component{"SymbolTableMap not yet defined."};
        }
        return *map_ptr;
    }

}