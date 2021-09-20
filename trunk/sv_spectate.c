#include "quakedef.h"

#include <assert.h>
#include <stdlib.h>


qboolean SV_IsSpectating(const client_t *client)
{
	return client->spec_state.repr != 0;
}

static int SV_GetSpectatedClientNum(const client_t *client)
{
	return abs(client->spec_state.repr) - 1;
}

client_t* SV_GetSpectatedClient(const client_t *client)
{
	if (client->spec_state.repr == 0 ||
	    abs(client->spec_state.repr) > svs.maxclients)
		return NULL;
	return svs.clients + SV_GetSpectatedClientNum(client);
}

qboolean SV_SetSpectator(client_t *client)
{
	const int i_self = client - svs.clients;
	assert(i_self >= 0 && i_self < svs.maxclients);

	for (int i = 0; i != svs.maxclients; ++i)
	{
		if (i != i_self && svs.clients[i].active && !SV_IsSpectating(&svs.clients[i]))
		{
			client->spec_state.repr = -(i + 1);
			SV_BroadcastPrintf ("%s is now spectating\n", client->name);
			return true;
		}
	}
	Con_Printf ("No one to spectate for %s\n", client->name);
	return false;
}

void SV_UnsetSpectator(client_t *client)
{
	client->spec_state.repr = 0;
	SV_BroadcastPrintf ("%s is not spectating anymore\n", client->name);
}

void SV_SpectateNext(client_t *client)
{
	assert(SV_IsSpectating(client));

	const int i_self = client - svs.clients;
	const int i_currently_following = SV_GetSpectatedClientNum(client);
	for (int i = i_currently_following + 1; i != i_currently_following; ++i)
	{
		if (i == svs.maxclients)
			i = 0;

		if (i != i_self && svs.clients[i].active && !SV_IsSpectating(&svs.clients[i]))
		{
			client->spec_state.repr = -(i + 1);
			return;
		}
	}
}

void SV_SpectatePrevious(client_t *client)
{
	assert(SV_IsSpectating(client));

	const int i_self = client - svs.clients;
	const int i_currently_following = SV_GetSpectatedClientNum(client);
	for (int i = i_currently_following - 1; i != i_currently_following; --i)
	{
		if (i == -1)
			i = svs.maxclients - 1;

		if (i != i_self && svs.clients[i].active && !SV_IsSpectating(&svs.clients[i]))
		{
			client->spec_state.repr = -(i + 1);
			return;
		}
	}
}

void SV_HandleNewSpectatedClient(client_t *client)
{
	if (client->spec_state.repr >= 0)
		return;

	SV_ClientPrintf ("Now spectating %s\n", SV_GetSpectatedClient(client)->name);
	client->spec_state.repr = -client->spec_state.repr;
}
