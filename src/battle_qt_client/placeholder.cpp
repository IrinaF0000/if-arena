// Placeholder for future Qt Widgets client target.
#include "network/NetworkClient.hpp"

int main()
{
	if_arena::battle_qt_client::network::NetworkClient client;
	return client.canSendIntent() ? 1 : 0;
}
