--==========================================================================
-- Tupsources.lua for ObTools build system
--
-- Builds sources variables
--
-- Copyright (c) 2016 Paul Clark. All rights reserved
--==========================================================================

SOURCES = {}
TESTSOURCES = {}

function add_for_dir(dir)
  local glob = '*.cc'
  if dir ~= "." then
    glob = dir .. '/*.cc'
  end
  for index, filename in ipairs(tup.glob(glob)) do
    local basename = string.match(filename, '([^/]+).cc$')
    if string.sub(basename, 1, 1) ~= '.' then
      if string.sub(basename, 1, 5) == 'test-' then
        TESTSOURCES += filename
      elseif string.sub(basename, 1, 7) ~= 'legacy-' then
        if basename ~= 'main' then
          SOURCES += filename
        end
      end
    end
  end
end

add_for_dir(".")
SUBS += SUBDIRS
for index, dir in ipairs(SUBS) do
  add_for_dir(dir)
end
