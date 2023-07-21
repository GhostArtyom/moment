 function output = cat(join_dimension, varargin)
    % Trivial cases:
    if nargin == 1
        output = MTKObject.empty(0,0);
        return;
    elseif nargin == 2
        output = varargin{1};
        return;
    end
    
    % Semi-trivial cases (prune empty arrays!)
    non_empty_mask = ~cellfun(@isempty, varargin);
    if ~any(non_empty_mask(:))
        output = MTKObject.empty(0,0);
        return;
    end
    varargin = varargin(non_empty_mask);
    if numel(varargin) == 1
        output = varargin(1);
        return;
    end

    % Ensure all inputs are ComplexObjects
    is_object = cellfun(@(x) isa(x, 'MTKObject'), varargin);
    if ~all(is_object(:))
        error("Can only concatenate MTKObjects");
    end
    
    % Disable concatenation from different scenarios
    matching_scenario = cellfun(@(x) (varargin{1}.Scenario == x.Scenario), varargin);
    if ~all(matching_scenario(:))
        error("Can only concatenate MTKObjects from the same scenario.");
    end

    % Disable hetrogenous concatenation
    homogenous = cellfun(@(x) strcmp(class(x), class(varargin{1})), varargin);
    if ~all(homogenous(:))
        % TODO: Cast to polynomial, and concatenate
        error("Can only concatenate MTKObjects of the same type.");
    end
    class_name = class(varargin{1});

    % Get output dimensions (fail if inconsistent)
    sizes = cellfun(@size, varargin, 'UniformOutput', false);
    matching_tensor = (numel(sizes{1}) == cellfun(@(x) numel(x), sizes));
    if ~all(matching_tensor(:))
        error("Cannot merge tensors of different dimensionality.");
    end

    % Check matching sizes on dimensions that are not joined 
    nonjoin_dimensions = [1:(join_dimension-1), ...
                          (join_dimension+1):numel(sizes{1})];
    cat_sizes = cellfun(@(x) x(join_dimension), sizes);
    non_cat_sizes = cellfun(@(x) x(nonjoin_dimensions), ...
                            sizes, 'UniformOutput', false);            
    consistent = cellfun(@(x) (isequal(non_cat_sizes{1}, x)), non_cat_sizes);
    if ~all(consistent(:))
        if numel(nonjoin_dimensions) == 1
            if join_dimension == 1
                error("Cannot vertically concatenate objects with different column sizes.");
            else
                error("Cannot horizontally concatenate objects with different row sizes.");                        
            end
        else
            error("Cannot concatenate objects with mismatched dimensions.");
        end
    end

    % Construct target size
    target_size = zeros(1, numel(sizes{1}));
    target_size(join_dimension) = sum(cat_sizes);
    [target_size(nonjoin_dimensions)] = non_cat_sizes{1};

    % Construct object offsets
    offsets = ones(numel(sizes{1}), numel(varargin));
    offsets(join_dimension, 1:end) = cumsum(cat_sizes);

    % Polymorphic c'tor
    output = feval(class_name + ".InitForOverwrite", ...
                   varargin{1}.Scenario, target_size);

    % Do merge
    output.mergeIn(join_dimension, offsets, varargin);

end
