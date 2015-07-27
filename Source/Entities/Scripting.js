
$predicate("$child is a $base", function(child, base)
{
	if (typeof base === 'string')
		base = Archive.find(base);

	if (!base)
		return false;

	var test = child;
	while (test)
	{
		if (test === base)
			return true;

		test = test.$baseObj;
	}

	return false;
});