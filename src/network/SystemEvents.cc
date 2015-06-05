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

#include "System.h"

namespace net
{

esim::EventType *System::ev_net_send;
esim::EventType *System::ev_net_output_buffer;
esim::EventType *System::ev_net_input_buffer;
esim::EventType *System::ev_net_receive;


void System::evNetSendHandler(esim::EventType *type, esim::EventFrame *frame)
{
}
	

void System::evNetOutputBufferHandler(esim::EventType *type, esim::EventFrame *frame)
{
}


void System::evNetInputBufferHandler(esim::EventType *type, esim::EventFrame *frame)
{
}


void System::evNetReceive(esim::EventType *type, esim::EventFrame *frame)
{
}

}

