/**
 * inflation_matrix_system.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "inflation_matrix_system.h"

#include "canonical_observables.h"
#include "factor_table.h"
#include "extension_suggester.h"

#include "extended_matrix.h"
#include "inflation_context.h"
#include "inflation_explicit_symbols.h"
#include "inflation_implicit_symbols.h"

namespace Moment::Inflation {
    InflationMatrixSystem::InflationMatrixSystem(std::unique_ptr<class InflationContext> contextIn)
            : MatrixSystem{std::move(contextIn)},
              inflationContext{dynamic_cast<class InflationContext&>(this->Context())} {
        this->factors = std::make_unique<FactorTable>(this->inflationContext, this->Symbols());
        this->canonicalObservables = std::make_unique<class CanonicalObservables>(this->inflationContext);
        this->extensionSuggester = std::make_unique<ExtensionSuggester>(this->inflationContext,
                                                                        this->Symbols(), *this->factors);
    }

    InflationMatrixSystem::InflationMatrixSystem(std::unique_ptr<class Context> contextIn)
            : MatrixSystem{std::move(contextIn)},
              inflationContext{dynamic_cast<class InflationContext&>(this->Context())} {
        this->factors = std::make_unique<FactorTable>(this->inflationContext, this->Symbols());
        this->canonicalObservables = std::make_unique<class CanonicalObservables>(this->inflationContext);
        this->extensionSuggester = std::make_unique<ExtensionSuggester>(this->inflationContext,
                                                                        this->Symbols(), *this->factors);
    }

    InflationMatrixSystem::~InflationMatrixSystem() noexcept = default;

    const InflationExplicitSymbolIndex &InflationMatrixSystem::ExplicitSymbolTable() const {
        if (!this->explicitSymbols) {
            throw Moment::errors::missing_component("ExplicitSymbolTable has not yet been generated.");
        }
        return *this->explicitSymbols;
    }

    const InflationImplicitSymbols &InflationMatrixSystem::ImplicitSymbolTable() const {
        if (!this->implicitSymbols) {
            throw Moment::errors::missing_component("ImplicitSymbolTable has not yet been generated.");
        }
        return *this->implicitSymbols;
    }

    size_t InflationMatrixSystem::MaxRealSequenceLength() const noexcept {
        // Largest order of moment matrix?
        ptrdiff_t hierarchy_level = this->highest_moment_matrix();
        if (hierarchy_level < 0) {
            hierarchy_level = 0;
        }

        // Max sequence can't also be longer than number of observable variants
        return std::min(hierarchy_level*2, static_cast<ptrdiff_t>(this->inflationContext.observable_variant_count()));
    }


    void InflationMatrixSystem::onNewMomentMatrixCreated(size_t level, const class MomentMatrix& mm) {
        // Register factors
        this->factors->on_new_symbols_added();

        // Update canonical observables (if necessary)
        const auto new_max_length = this->MaxRealSequenceLength();
        this->canonicalObservables->generate_up_to_level(new_max_length);

        // Update explicit/implicit symbols
        if (!this->explicitSymbols || (this->explicitSymbols->Level < new_max_length)) {
            this->explicitSymbols = std::make_unique<InflationExplicitSymbolIndex>(*this, new_max_length);
            this->implicitSymbols = std::make_unique<InflationImplicitSymbols>(*this);
        }

        MatrixSystem::onNewMomentMatrixCreated(level, mm);
    }

    void InflationMatrixSystem::onNewLocalizingMatrixCreated(const LocalizingMatrixIndex &lmi,
                                                             const class LocalizingMatrix& lm) {
        // Register factors
        this->factors->on_new_symbols_added();
        MatrixSystem::onNewLocalizingMatrixCreated(lmi, lm);
    }

    ptrdiff_t InflationMatrixSystem::find_extended_matrix(size_t mm_level, std::span<const symbol_name_t> extensions) {
        // Do we have any extended matrices at this MM level?
        const auto* indexRoot = this->extension_indices.find_node(static_cast<symbol_name_t>(mm_level));
        if (nullptr == indexRoot) {
            return -1;
        }

        // Try with extensions
        auto index = indexRoot->find(extensions);
        if (!index.has_value()) {
            return -1;
        }
        return static_cast<ptrdiff_t>(index.value());
    }

    std::pair<size_t, ExtendedMatrix &>
    InflationMatrixSystem::create_extended_matrix(const class MomentMatrix &source,
                                                  std::span<const symbol_name_t> extensions) {
        auto lock = this->get_write_lock();

        // Attempt to get pre-existing extended matrix
        auto pre_existing = this->find_extended_matrix(source.Level(), extensions);
        if (pre_existing >= 0) {
            auto& existingMatrix = this->get(pre_existing);
            return {pre_existing, dynamic_cast<ExtendedMatrix&>(existingMatrix)};
        }

        // ...otherwise, create new one.
        auto em_ptr = std::make_unique<ExtendedMatrix>(this->Symbols(), this->Factors(), source, extensions);
        auto& ref = *em_ptr;
        auto index = this->push_back(std::move(em_ptr));

        // Register index in tree
        auto * root = this->extension_indices.add_node(static_cast<symbol_name_t>(source.Level()));
        root->add(extensions, index);

        // Return created matrix
        return {index, ref};
    }

    std::set<symbol_name_t> InflationMatrixSystem::suggest_extensions(const class MomentMatrix& matrix) const {
        return (*this->extensionSuggester)(matrix);
    }

}