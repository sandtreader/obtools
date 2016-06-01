--==========================================================================
-- Tupdepends.lua for ObTools build system
--
-- Builds dependency variable using recursive lookup of dependencies
--
-- Copyright (c) 2016 Paul Clark. All rights reserved
--==========================================================================

tup.include('Tuppath.lua')

-- bring DEPENDS into our context (+= is a custom tup function that has access
-- to the tup context)
DEPS += DEPENDS

-- get full depends by recursively looking in other Tupfiles
FULLDEPENDS = {};

function get_deps(name)
  if FULLDEPENDS[name] == nil then
    local libdir = get_dependency_path(name)
    if libdir ~= nil then
      local f = io.open(libdir .. '/Tupfile', 'r')
      if f == nil then
        error("Could not open Tupfile in " .. libdir)
      end
      for line in f:lines() do
        local d = line:match('DEPENDS[ ]*=(.*)')
        if d ~= nil then
          for n in string.gmatch(d, '([^ ]+)') do
            get_deps(n)
          end
        end
      end
      f:close()
    end
    FULLDEPENDS[name] = 1
  end
end

if not (next(DEPS) == nil) then
  for index, dep in pairs(DEPS) do
    get_deps(dep)
  end
end

-- sort the full dependencies so they product a consistant order for command
-- line options
FULLDEPS={}
for dep, value in pairs(FULLDEPENDS) do
  table.insert(FULLDEPS, dep)
end
table.sort(FULLDEPS)

-- Finally, create the output variables
DEPINCLUDES = {};
DEPLINKS = {};
DEPGROUPS = {}
for index, dep in ipairs(FULLDEPS) do
  if string.sub(dep, 1, 4) == 'ext-' then
    -- external, so just link libs
    DEPLINKS += '-l' .. string.sub(dep, 5)
  else
    local libdir = tup.getcwd() .. '/' .. get_dependency_path(dep)
    if libdir ~= nil then
      DEPINCLUDES += '-I' .. libdir
      DEPLINKS += '%<' .. dep .. '>'
      DEPGROUPS += libdir .. '/<' .. dep .. '>'
    end
  end
end
