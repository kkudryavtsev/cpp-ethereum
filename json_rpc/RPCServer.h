/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.	If not, see <http://www.gnu.org/licenses/>.
*/
/** @file RPCServer.h
 * @author Konstantin Kudryavtsev <konstantin@ethertools.net>
 * @date 2014
 * Ethereum client.
 */

#pragma once

#include <jsonrpc/rpc.h>
#include "Client.h" 

namespace eth
{

	class RPCServer : public jsonrpc::AbstractServer<RPCServer>
	{
	public:

		RPCServer();
		void getLastBlock(const Json::Value&, Json::Value&);
		void getBlock(const Json::Value&, Json::Value&);
		void getAddress(const Json::Value&, Json::Value&);
		void makeTransaction(const Json::Value&, Json::Value&);

	private:
		Client m_client;
		Json::Value getTransactions(bytesConstRef);
	};

}
