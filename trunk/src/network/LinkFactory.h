/*
 *  Multi2Sim
 *  Copyright (C) 2014  Amir Kavyan Ziabari (aziabari@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef NETWORK_LINKFACTORY_H
#define NETWORK_LINKFACTORY_H

#include <memory>

#include "Link.h"

namespace misc
{
class IniFile;
}

namespace net
{

class LinkFactory
{
public:

	/// Produce a Link by INI file section
	virtual std::unique_ptr<Link> ProduceLinkByIniSection(Network *network,
			const std::string &section, misc::IniFile &config);

	/// Produce a node by type string
	virtual std::unique_ptr<Link> ProduceLink(Network *network,
			const std::string &name, 
			Node *source_node, Node *destination_node);

};

}  // namespace net

#endif
