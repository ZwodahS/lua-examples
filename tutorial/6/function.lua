if functions == nil then functions = {}; end
if functions.computation == nil then functions.computation = {}; end

-- create a local reference to functions.computation.
local package = functions.computation;

-- don't put any of these in the global namspace.
function package.compute(x, y)
    return x - y
end
function package.multi_compute(x, y, z)
    return x + y, y + z, x + z
end

