'use strict';

/* exported EventDispatcher */
class EventDispatcher 
{
	constructor()
	{
		this.eventMap = null;
	}

	addListener(type, handler, receipt)
	{
		if (this.eventMap === null)
			this.eventMap = new Map();

		var handlers = this.eventMap.get(type);
		if (!handlers)
		{
			handlers = [];
			this.eventMap.set(type, handlers);
		}

		if (handlers.indexOf(handler) < 0)
			handlers.push(handler);

		if (receipt !== null)
			receipt.add(this, type, handler);
	}

	removeListener(type, handler)
	{
		if (this.eventMap === null)
			return;

		var handlers = this.eventMap.get(type);
		if (handlers)
		{
			var idx = handlers.indexOf(handler);
			if (idx >= 0)
			{
				handlers[idx] = null;
			}
		}
	}

	waitFor(...types)
	{
		return new Promise((resolve, reject) =>
		{
			let receipt = new ExclusiveEventReceipt();
			let handler = (e) => 
			{
				receipt.clear();
				resolve(e);
			};

			for (let type of types)
			{
				this.addListener(type, handler, receipt);
			}
		});
	}

	dispatch(e)
	{
		if (this.eventMap === null)
			return;

		var handlers = this.eventMap.get(e.constructor);
		if (handlers)
		{
			for (var i = 0; i < handlers.length; i++) 
			{
				var handler = handlers[i];
				if (handler === null)
				{
					handlers.splice(i, 1);
					i--;
				}
				else
				{
					handler(e);
				}
			}
		}
	}
}

// multiple sources
// multiple events
// single handler per event
class CollectiveEventReceipt
{
	constructor()
	{
		this.entries = [];
	}

	add(source, type, handler)
	{
		for (let i = 0; i < this.entries.length; i++) 
		{
			let entry = this.entries[i];
			if (entry.source === source &&
				entry.type === type)
			{
				if (entry.handler !== handler)
				{
					source.removeListener(type, entry.handler);
					entry.handler = handler;
				}

				return;
			}
		}

		this.entries.push(
		{
			source: source,
			type: type,
			handler: handler
		});
	}

	clear()
	{
		for (var i = 0; i < this.entries.length; i++) 
		{
			var entry = this.entries[i];
			entry.source.removeListener(entry.type, entry.handler);
		}

		this.entries.length = 0;
	}
}

// Single source
// multiple events
// Single handler per event
class ExclusiveEventReceipt
{
	constructor()
	{
		this.source = null;
		this.entries = [];
	}

	add(source, type, handler)
	{
		if (source === null)
			throw ("source cannot be null.");

		if (this.source !== source)
		{
			this.clear();
			this.source = source;
		}
		else
		{
			for (var i = this.entries.length - 1; i >= 0; i--) 
			{
				var entry = this.entries[i];
				if (entry.type === type)
				{
					if (entry.handler !== handler)
					{
						source.removeListener(type, entry.handler);
						entry.handler = handler;
					}

					return;
				}
			}			
		}

		this.entries.push(
		{
			type: type,
			handler: handler
		});
	}

	clear()
	{
		if (this.source !== null)
		{
			for (var i = 0; i < this.entries.length; i++) 
			{
				var entry = this.entries[i];
				this.source.removeListener(entry.type, entry.handler);
			}
		}

		this.source = null;
		this.entries.length = 0;
	}
}

