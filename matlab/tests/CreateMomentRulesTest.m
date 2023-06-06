classdef CreateMomentRulesTest < MTKTestBase
    %APPLYVALUESTEST Unit tests for apply_values function
    methods (Test)
        function SubList_Empty(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', ref_id, {});
            rules = mtk('create_moment_rules', 'info', 'output', 'symbols', ref_id, rules_index);
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.verifyTrue(isempty(rules));
            rules_index2 = mtk('create_moment_rules', ref_id, {});
            testCase.verifyEqual(rules_index2, uint64(1));
        end
        
        function SubList_Simple(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', ...
                             ref_id, {{2, 0.5}});
            rules = mtk('create_moment_rules', 'info', ...
                        'output', 'symbols', ref_id, rules_index);
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{uint64(2), {{uint64(1), 0.5}}}});
        end
        
        function SubList_Multi(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', ref_id, ...
                              {{2, 0.3}, {3, 0.4}});
            rules = mtk('create_moment_rules', 'info', ...
                        'output', 'symbols', ref_id, rules_index);
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{uint64(2), {{uint64(1), 0.3}}}; ...
                 {uint64(3), {{uint64(1), 0.4}}}});
        end
        
          function Symbols_Empty(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', ref_id,...
                              'input', 'symbols', {});
            rules = mtk('create_moment_rules', 'info', ...
                        'output', 'symbols', ref_id, rules_index);
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.verifyTrue(isempty(rules));            
        end
        
        function Symbols_Simple(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', 'input', 'symbols',...
                              ref_id, {{{2, -1.0}, {1, 0.5}}});
            rules = mtk('create_moment_rules', 'info', ...
                        'output', 'symbols', ref_id, rules_index);
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{uint64(2), {{uint64(1), 0.5}}}});
        end
        
        function Symbols_Multi(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', 'input', 'symbols',... 
                               ref_id, ...
                               {{{2, -1.0}, {1, 0.3}};...
                                {{3, -1.0}, {1, 0.4}}});
            rules = mtk('create_moment_rules', 'info', ...
                        'output', 'symbols', ref_id, rules_index);                                    
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{uint64(2), {{uint64(1), 0.3}}}; ...
                 {uint64(3), {{uint64(1), 0.4}}}});
        end
                
        function Symbols_Chain(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', 'input', 'symbols',...
                              ref_id, ...
                              {{{2, -1.0}, {1, 0.3}}; ...
                               {{3, -1.0}, {2, 0.4}}});
            rules = mtk('create_moment_rules', 'info', ...
                        'output', 'symbols', ref_id, rules_index);
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{uint64(2), {{uint64(1), 0.3}}}; ...
                 {uint64(3), {{uint64(1), 0.12}}}});
        end
        
        function OpSeq_Existing_Empty(testCase)
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', ref_id, ...
                             'input', 'sequences', 'no_new_symbols', {});
            rules = mtk('create_moment_rules', 'info', ...
                        'output', 'symbols', ref_id, rules_index);
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.verifyTrue(isempty(rules));   
        end
        
         function OpSeq_Existing_Simple(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', ref_id, ...
                              'input', 'sequences', 'no_new_symbols', ...
                              {{{[2], -1.0}, {[1], 0.5}}});
            rules = mtk('create_moment_rules', 'info', ...
                        'output', 'symbols', ref_id, rules_index);
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{uint64(3), {{uint64(2), 0.5}}}}); % Rule is A1 -> 0.5 A0
         end
        
         
        function OpSeq_Existing_Chain(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', ref_id, ...
                              'input', 'sequences', 'no_new_symbols', ...
                              {{{[1], -1.0}, {[], 0.3}}; ...
                               {{[2], -1.0}, {[1], 0.4}}});
            rules = mtk('create_moment_rules', 'info', ...
                       'output', 'symbols', ref_id, rules_index);
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{uint64(2), {{uint64(1), 0.3}}}; ...
                 {uint64(3), {{uint64(1), 0.12}}}});
        end
        
        
        function OpSeq_NewSymbols_Empty(testCase)
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            old_symbols = mtk('symbol_table', ref_id);
            testCase.verifyEqual(length(old_symbols), 2);
            rules_index = mtk('create_moment_rules', ref_id, ...
                              'input', 'sequences', {});                    
            rules = mtk('create_moment_rules', 'info', ...
                       'output', 'symbols', ref_id, rules_index);      
            new_symbols = mtk('symbol_table', ref_id);
            testCase.verifyEqual(length(new_symbols), 2);
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.verifyTrue(isempty(rules));   
        end
        
         function OpSeq_NewSymbols_Simple(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            old_symbols = mtk('symbol_table', ref_id);
            testCase.verifyEqual(length(old_symbols), 2);
            rules_index = mtk('create_moment_rules', ref_id, ...
                              'input', 'sequences', 'order', 'hash', ...
                              {{{[2], -1.0}, {[1], 0.5}}});
            rules = mtk('create_moment_rules', 'info', ...
                       'output', 'symbols', ref_id, rules_index);
                   
            new_symbols = mtk('symbol_table', ref_id);
            testCase.verifyEqual(length(new_symbols), 4);
            
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{uint64(2), {{uint64(3), 0.5}}}}); % Rule is A1 -> 0.5 A0
            
         end
        
         
        function OpSeq_NewSymbols_Chain(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            old_symbols = mtk('symbol_table', ref_id);
            testCase.verifyEqual(length(old_symbols), 2);
            rules_index = mtk('create_moment_rules', ref_id, ...
                              'input', 'sequences', ...
                              {{{[1], -1.0}, {[], 0.3}}; ...
                               {{[2], -1.0}, {[1], 0.4}}});
            rules = mtk('create_moment_rules', 'info', ...
                       'output', 'symbols', ref_id, rules_index);                                    
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{uint64(2), {{uint64(1), 0.3}}}; ...
                 {uint64(3), {{uint64(1), 0.12}}}});
            new_symbols = mtk('symbol_table', ref_id);
            testCase.verifyEqual(length(new_symbols), 4);
        end
        
        function AppendRules_NoClash(testCase)
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', ref_id, {{2, 0.3}});
            second_index = mtk('create_moment_rules', ref_id, ...
                               'rulebook', rules_index, {{3, 0.4}});
            rules = mtk('create_moment_rules', 'info', ...
                       'output', 'symbols', ref_id, rules_index);
            testCase.verifyEqual(second_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{uint64(2), {{uint64(1), 0.3}}}; ...
                 {uint64(3), {{uint64(1), 0.4}}}});
        end
                 
        function WithLabelAndTolerance(testCase)            
            ref_id = mtk('locality_matrix_system', 2, 2, 2);
            [~] = mtk('moment_matrix', ref_id, 1);
            rules_index = mtk('create_moment_rules', 'input', 'symbols',...
                              'label', "Named rulebook", ...
                              'tolerance', 500, ...
                               ref_id, {{{2, -1.0}, {1, 0.5}}});
            rules = mtk('create_moment_rules', 'info', ...
                       'output', 'symbols', ref_id, rules_index);                                                                       
            testCase.verifyEqual(rules_index, uint64(0));
            testCase.assertFalse(isempty(rules));
            testCase.verifyEqual(rules, ...
                {{uint64(2), {{uint64(1), 0.5}}}});
        end
        
    end
    
   
    methods (Test, TestTags={'Error'})
        function Error_NoInput(testCase)
            function no_in()
                [~] = mtk('create_moment_rules');
            end
            testCase.verifyError(@() no_in(), 'mtk:too_few_inputs');
        end
        
        function Error_TooManyInputs(testCase)
            function bad_call()
                ref_id = mtk('locality_matrix_system', 2, 2, 2);
                [~] = mtk('create_moment_rules', ref_id, ref_id, ref_id);
            end
            testCase.verifyError(@() bad_call(), 'mtk:too_many_inputs');
        end
        
        function Error_BadMatrixSystem(testCase)
            function bad_call()
                ref_id = mtk('locality_matrix_system', 2, 2, 2);
                [~] = mtk('create_moment_rules', ref_id+1, {});
            end
            testCase.verifyError(@() bad_call(), 'mtk:bad_param');
        end
        
        function Error_SymbolNotFound_SubList(testCase)
            function bad_call()
                ref_id = mtk('locality_matrix_system', 2, 2, 2);
                [~] = mtk('create_moment_rules', ref_id, {{100, 1.0}});
            end
            testCase.verifyError(@() bad_call(), 'mtk:bad_param');
        end
        
        function Error_SymbolNotFound_SymbolPolynomial(testCase)
            function bad_call()
                ref_id = mtk('locality_matrix_system', 2, 2, 2);
                [~] = mtk('create_moment_rules', ref_id, 'input', 'symbols', ...
                             {{{100, 1.0}}});
            end
            testCase.verifyError(@() bad_call(), 'mtk:bad_param');
        end
        
        function Error_SymbolNotFound_SequenceNoCreate(testCase)
            function bad_call()
                ref_id = mtk('locality_matrix_system', 2, 2, 2);
                [~] = mtk('create_moment_rules', ref_id, ...
                             'input', 'sequences', 'no_new_symbols', ...
                             {{{[1 2], 1.0}}});
            end
            testCase.verifyError(@() bad_call(), 'mtk:bad_param');
        end
    end
end
