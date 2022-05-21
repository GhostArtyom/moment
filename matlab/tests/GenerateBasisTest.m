classdef GenerateBasisTest < matlab.unittest.TestCase
    %GENERATEBASISTEST Unit tests for generate basis function
    
    properties(Constant)
        dense_input = [[1, 2, 3]; [2, 4, -3]; [3, -3, 5]]
        sparse_input = sparse([[1, 2, 3]; [2, 4, -3]; [3, -3, 5]])
        
        dense_expected_re = {[[1,0,0]; [0,0,0]; [0,0,0]], ... 
                              [[0,1,0]; [1,0,0]; [0,0,0]], ...
                              [[0,0,1]; [0,0,-1]; [1,-1,0]], ...
                              [[0,0,0]; [0,1,0]; [0,0,0]], ...
                              [[0,0,0]; [0,0,0]; [0,0,1]]}
                          
       dense_expected_im = {0, [[0,1i,0]; [-1i,0,0]; [0,0,0]], ...
                            [[0,0,1i]; [0,0,-1i]; [-1i,1i,0]], 0, 0}
                        
       sparse_expected_re = {sparse([[1,0,0]; [0,0,0]; [0,0,0]]), ... 
                             sparse([[0,1,0]; [1,0,0]; [0,0,0]]), ...
                             sparse([[0,0,1]; [0,0,-1]; [1,-1,0]]), ...
                             sparse([[0,0,0]; [0,1,0]; [0,0,0]]), ...
                             sparse([[0,0,0]; [0,0,0]; [0,0,1]])}
                          
       sparse_expected_im = {0, sparse([[0,1i,0]; [-1i,0,0]; [0,0,0]]), ...
                            sparse([[0,0,1i]; [0,0,-1i]; [-1i,1i,0]]), ...
                            0, 0}
    end
    
    methods(TestMethodSetup)
        function addNPATKpath(testCase)
            import matlab.unittest.fixtures.PathFixture
            testCase.applyFixture(PathFixture([".."]));
        end
    end
    
    methods(TestMethodTeardown)
        function clearNPATK(testCase)
            clear npatk
        end
    end
    
    methods
        function verify_sym_output(testCase, output, keys, sparse)
            testCase.assertSize(keys, [5, 2])
            testCase.assertLength(output, 5)
            
            if sparse
                compare_to = testCase.sparse_expected_re;
            else
                compare_to = testCase.dense_expected_re;
            end
            
            % check referred to basis elements match expectations
            for match = 1:5
                index = keys(keys(:,1)==match,2);
                testCase.assertNumElements(index, 1)
                testCase.assertGreaterThan(index, 0)
                testCase.verifyEqual(output{index}, ...
                    compare_to{match})
            end
        end
        
         function verify_herm_output(testCase, output_re, output_im, ...
                                     keys, sparse)
            testCase.assertSize(keys, [5, 3])
            testCase.assertLength(output_re, 5)
            testCase.assertLength(output_im, 2)
            
            
            if sparse
                compare_to_re = testCase.sparse_expected_re;
                compare_to_im = testCase.sparse_expected_im;
            else
                compare_to_re = testCase.dense_expected_re;
                compare_to_im = testCase.dense_expected_im;
            end
            
            % check referred to symmetric basis elements match expectations
            for match = 1:5
                index = keys(keys(:,1)==match,2);
                testCase.assertNumElements(index, 1)
                testCase.assertGreaterThan(index, 0)
                testCase.verifyEqual(output_re{index}, ...
                    compare_to_re{match})
            end
            
            % entries 1, 4 and 5 have no imaginary part:
            for match = [1, 4, 5]
                index = keys(keys(:,1)==match,3);
                testCase.assertNumElements(index, 1)
                testCase.verifyEqual(double(index), 0)
            end
            
            % but check for anti-symmetric basis entries of 2 and 3            
            for match = [2, 3]
                index = keys(keys(:,1)==match,3);
                testCase.assertNumElements(index, 1)
                testCase.assertGreaterThan(index, 0)
                testCase.verifyEqual(output_im{index}, ...
                    compare_to_im{match})
            end 
        end
        
    end
    
    methods (Test)
        function dense_from_dense_symmetric(testCase)
            [output, keys] = npatk('generate_basis', ...
                'dense', 'symmetric', testCase.dense_input);
            testCase.verify_sym_output(output, keys, false);
        end 
        
         function dense_from_dense_hermitian(testCase)
            [output_re, output_im, keys] = npatk('generate_basis', ...
                'dense', 'hermitian', testCase.dense_input);
            testCase.verify_herm_output(output_re, output_im, keys, false);
         end 
        
         function dense_from_sparse_symmetric(testCase)
            [output, keys] = npatk('generate_basis', ...
                'dense', 'symmetric', testCase.sparse_input);
            testCase.verify_sym_output(output, keys, false);
        end 
        
         function dense_from_sparse_hermitian(testCase)
            [output_re, output_im, keys] = npatk('generate_basis', ...
                'dense', 'hermitian', testCase.sparse_input);
            testCase.verify_herm_output(output_re, output_im, keys, false);
         end 
        
        function sparse_from_dense_symmetric(testCase)
            [output, keys] = npatk('generate_basis', ...
                'sparse', 'symmetric', testCase.dense_input);
            testCase.verify_sym_output(output, keys, true);
        end 
        
         function sparse_from_dense_hermitian(testCase)
            [output_re, output_im, keys] = npatk('generate_basis', ...
                'sparse', 'hermitian', testCase.dense_input);
            testCase.verify_herm_output(output_re, output_im, keys, true);
         end 
        
         function sparse_from_sparse_symmetric(testCase)
            [output, keys] = npatk('generate_basis', ...
                'sparse', 'symmetric', testCase.sparse_input);
            testCase.verify_sym_output(output, keys, true);
        end 
        
         function sparse_from_sparse_hermitian(testCase)
            [output_re, output_im, keys] = npatk('generate_basis', ...
                'sparse', 'hermitian', testCase.sparse_input);
            testCase.verify_herm_output(output_re, output_im, keys, true);
        end 
    end
end

