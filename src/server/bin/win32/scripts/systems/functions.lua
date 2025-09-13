function getSystemData(system_key)
	if Systems[system_key] then
		return Systems[system_key]
	end
	return nil
end

function getSiteData(system_key, site_key)
	local system = getSystemData(system_key)
	
	if system then
		local site = system.Sites[site_key]
		if site then
			return site
		end
	end
	return nil
end