classdef ImportedScenario < Abstract.Scenario
%IMPORTEDSCENARIO Scenario for manually input moment matrices.
% 
% Unlike the other scenarios, moment and localizing matrices are not 
% generated automatically, as no fundamental operators are defined by
% this scenario. Instead, matrices can be registered and added using 
% the ImportMatrix method. 
%
% This allow for Moment functionality, such as the generation of bases
% for use with solvers, with moment matrices generated by other
% means.

% EXAMPLES:
%       /examples/imported_matrices.m
%    
    properties(GetAccess = public, SetAccess = protected)
        Real % True if every definable symbol in the system is real.
    end
        
    %% Construction and initialization
    methods
        function obj = ImportedScenario(all_real)
        % IMPORTEDSCENARIO Construct an empty scenario, ready for import.
        %
        % PARAMS
        %   all_real - True if every definable symbol in the system is real.
        %
        arguments
            all_real (1,1) logical = false
        end
        
        if nargin == 0
        	all_real = false;
        end
            
            % Superclass c'tor
            obj = obj@Abstract.Scenario();
            
            % Set whether all symbols are real or not
            obj.Real = logical(all_real);            
        end
    end
    
    %% Virtual methods
    methods(Access={?Abstract.Scenario,?MatrixSystem})
        
        function ref_id = createNewMatrixSystem(obj)
        % CREATENEWMATRIXSYSTEM Invoke mtk to create imported matrix system.
            arguments
                obj (1,1) ImportedScenario
            end
            cell_args = cell.empty;
            if obj.Real
                cell_args{end+1} = 'real';
            end
            
            ref_id = mtk('new_imported_matrix_system', cell_args{:});
        end
    end
    
    %% Input methods
    methods
        function val = ImportMatrix(obj, input, matrix_type)
        % IMPORTMATRIX Incorporate symbolic matrix into matrix system.
        %
        % Any symbols that appear in previously imported matrices share the
        % same meaning. If the matrix_type specified forces the imaginary
        % (or real) part of a symbol to be zero, then this constraint is 
        % imposed everywhere the symbol appears. If, to be consistent, with
        % all the matrices it appears in, a symbol must be zero (and the
        % symbol is not itself "0"), then an error is raised.
        %
        % In particular, when matrix_type "hermitian" is selected, diagonal
        % elements will be interpretted as real-valued. Likewise, if
        % off-diagonal element at (i,j) and (j,i) are both set to X, then X
        % will be inferred to be real (to keep X complex, instead set (j,i)
        % to "X*").
        %
        % When matrix_type is "symmetric", the function expects (i,j)=(j,i) 
        % and will throw an error otherwise. It is recommended not to use 
        % "symmetric" with complex matrices, as this imposes the constraint 
        % M = transpose(M), as opposed to M = ctranspose(M).
        %
        % PARAMS
        %   input - The square matrix to import. Can be integer, numeric or
        %           string. Conjugation should be represented as string 
        %           "5*". Negation as either -5 or "-5".
        %   matrix_type - Symmetry constraints, and symbol types of the
        %                 matrix. Must be one of "real", "complex",
        %                 "symmetric" or "hermitian". 
        %
        % RETURNS
        %   An OpMatrix.OperatorMatrix, containing a handle to the newly
        %   imported matrix.
        %
        % See also: OpMatrix.OperatorMatrix
            arguments
                obj (1,1) ImportedScenario
                input
                matrix_type 
            end
            cell_args = cell.empty;
            
            % Validate input
            if nargin < 2
               error("A matrix must be supplied as input.");
            end
            if ~ismatrix(input)
                error("Input must be a matrix");
            end
            [dimension, other_dimension] = size(input);
            if other_dimension ~= dimension
                error("Input must be a square matrix.");
            end
            
            % Validate matrix type
            if nargin >= 3
                matrix_type = lower(char(matrix_type));
                if ~ismember(matrix_type, ...
                            {'real', 'complex', 'symmetric', 'hermitian'})
                    error("Matrix type must be real, complex, symmetric or hermitian.");
                end
                cell_args{end+1} = matrix_type;
                
            end
            index = mtk('import_matrix', obj.System.RefId, ...
                                     input, cell_args{:});

            val = OpMatrix.OperatorMatrix(obj.System, index, dimension);
            
            % Update symbols, with forced reset
            obj.System.UpdateSymbolTable(true);            
        end
        
        function val = ImportSymmetricMatrix(obj, input)
        % IMPORTSYMMETRICMATRIX Alias for ImportMatrix with matrix_type set to 'symmetric'.
        %
        % See also: IMPORTMATRIX
            arguments
                obj (1,1) ImportedScenario
                input
            end
            val = obj.ImportMatrix(input, 'symmetric');
        end
        
        function val = ImportHermitianMatrix(obj, input)
        % IMPORTSYMMETRICMATRIX Alias for ImportMatrix with matrix_type set to 'hermitian'.
        %
        % See also: IMPORTMATRIX
            arguments
                obj (1,1) ImportedScenario
                input
            end
            val = obj.ImportMatrix(input, 'hermitian');
        end
    end
    
    %% Virtual methods
    methods(Access=protected)
        function onNewMomentMatrix(obj, mm)
            arguments
                obj (1,1) ImportedScenario
                mm (1,1) OpMatrix.MomentMatrix
            end
            error('Imported scenario can not generate matrices.');
        end
    end
end

