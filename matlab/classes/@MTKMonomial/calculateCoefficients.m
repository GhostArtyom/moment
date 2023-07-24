 function [re, im] = calculateCoefficients(obj)

    % Early exit if we can't get symbol information...
    if ~all(obj.SymbolId(:) >= 0)
        error("Symbols are not yet all registered in matrix system.");
    end 

    % Real coefficients
    switch obj.DimensionType
        case 0 % SCALAR                    
            sys = obj.Scenario.System;
            re = zeros(double(sys.RealVarCount), 1, 'like', sparse(1i));
            if obj.re_basis_index > 0
                re(obj.re_basis_index) = obj.Coefficient;
            end
        otherwise
            re = makeMonolithicRealCoefficients(obj);            
    end

    % Imaginary coefficients
    switch obj.DimensionType
        case 0 % SCALAR
            sys = obj.Scenario.System;
			im = zeros(double(sys.ImaginaryVarCount), 1, 'like', sparse(1i));
            if obj.im_basis_index > 0
                if obj.SymbolConjugated
                    im(obj.im_basis_index) = -1i * obj.Coefficient;
                else
                    im(obj.im_basis_index) = 1i * obj.Coefficient;
                end
            end        
        otherwise             
             im = makeMonolithicImaginaryCoefficients(obj);
                   
                   
    end
 end

%% Private functions
function re = makeMonolithicRealCoefficients(obj)  
    nz_mask = obj.re_basis_index > 0;
    nz_cols = find(obj.re_basis_index > 0);
    nz_rows = double(obj.re_basis_index(nz_mask));
    
    re = sparse(nz_rows, nz_cols, obj.Coefficient(nz_mask), ...
                double(obj.Scenario.System.RealVarCount), numel(obj));
            
    % Hack for old version of matlab where complex(sparse(...)) fails.
    zero = zeros(double(obj.Scenario.System.RealVarCount), 0, ...
                 'like', sparse(1i));
    re = [zero, re];

end

function im = makeMonolithicImaginaryCoefficients(obj)
    nz_mask = obj.im_basis_index > 0;
    nz_cols = find(obj.im_basis_index > 0);
    nz_rows = double(obj.im_basis_index(nz_mask));
    
    values = (1i * ~obj.symbol_conjugated(nz_mask)) + ...
        -1i * obj.symbol_conjugated(nz_mask);
    values = values .* obj.Coefficient(nz_mask);
    
    im = sparse(nz_rows, nz_cols, values, ...
                double(obj.Scenario.System.ImaginaryVarCount), numel(obj));
    
    % Hack for old version of matlab where complex(sparse(...)) fails.
    zero = zeros(double(obj.Scenario.System.ImaginaryVarCount), 0, ...
                 'like', sparse(1i));
    im = [zero, im];

end
