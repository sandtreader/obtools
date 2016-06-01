--==========================================================================
-- Tupsources.lua for ObTools build system
--
-- Builds sources variables
--
-- Copyright (c) 2016 Paul Clark. All rights reserved
--==========================================================================

SOURCES = {}
TESTSOURCES = {}
for index, filename in ipairs(tup.glob('*.cc')) do
  if string.sub(filename, 1, 1) ~= '.' then
    if string.sub(filename, 1, 5) == 'test-' then
      TESTSOURCES += filename
    elseif string.sub(filename, 1, 7) ~= 'legacy-' then
      SOURCES += filename
    end
  end
end
