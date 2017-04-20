function get_dependency_path(name)
  if name == 'ot-xmlmesh' then
    return 'xmlmesh/core'
  elseif name == 'ot-xmlmesh-send' then
    return 'xmlmesh/bindings/cli/send'
  elseif name == 'ot-xmlmesh-receive' then
    return 'xmlmesh/bindings/cli/receive'
  elseif string.sub(name, 1, 11) == 'ot-xmlmesh-' then
    return 'xmlmesh/' .. string.sub(name, 12)
  elseif name == 'ot-toolgen' then
    return 'tools/toolgen'
  elseif string.sub(name, 1, 11) == 'ot-obcache-' then
    return 'obcache/libs/' .. string.sub(name, 12)
  elseif string.sub(name, 1, 3) == 'ot-' then
    return 'libs/' .. string.sub(name, 4)
  end
  return nil
end
