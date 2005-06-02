/*       +------------------------------------+
 *       | Inspire Internet Relay Chat Daemon |
 *       +------------------------------------+
 *
 *  Inspire is copyright (C) 2002-2004 ChatSpike-Dev.
 *                       E-mail:
 *                <brain@chatspike.net>
 *           	  <Craig@chatspike.net>
 *     
 * Written by Craig Edwards, Craig McLure, and others.
 * This program is free but copyrighted software; see
 *            the file COPYING for details.
 *
 * ---------------------------------------------------
 */

using namespace std;

#include <stdio.h>
#include "users.h"
#include "channels.h"
#include "modules.h"
#include <string>
#include "helperfuncs.h"
#include "hashcomp.h"

/* $ModDesc: Povides support for services +r user/chan modes and more */

class ModuleServices : public Module
{
	Server *Srv; 
 public:
	ModuleServices()
	{
		Srv = new Server;

		Srv->AddExtendedMode('r',MT_CHANNEL,false,0,0);
		Srv->AddExtendedMode('r',MT_CLIENT,false,0,0);
		Srv->AddExtendedMode('R',MT_CHANNEL,false,0,0);
		Srv->AddExtendedMode('R',MT_CLIENT,false,0,0);
		Srv->AddExtendedMode('M',MT_CHANNEL,false,0,0);
	}

        virtual void On005Numeric(std::string &output)
        {
                std::stringstream line(output);
                std::string temp1, temp2;
                while (!line.eof())
                {
                        line >> temp1;
                        if (temp1.substr(0,10) == "CHANMODES=")
                        {
                                // append the chanmode to the end
                                temp1 = temp1.substr(10,temp1.length());
                                temp1 = "CHANMODES=" + temp1 + "rRM";
                        }
                        temp2 = temp2 + temp1 + " ";
                }
		if (temp2.length())
	                output = temp2.substr(0,temp2.length()-1);
        }
	
	virtual int OnExtendedMode(userrec* user, void* target, char modechar, int type, bool mode_on, string_list &params)
	{
		
		if (modechar == 'r')
  		{
			if (type == MT_CHANNEL)
			{
				// only a u-lined server may add or remove the +r mode.
				if ((Srv->IsUlined(user->nick)) || (Srv->IsUlined(user->server)) || (!strcmp(user->server,"") || (strchr(user->nick,'.'))))
				{
					log(DEBUG,"Allowing umode +r, server and nick are: '%s','%s'",user->nick,user->server);
					return 1;
				}
				else
				{
					log(DEBUG,"Only a server can set chanmode +r, server and nick are: '%s','%s'",user->nick,user->server);
					Srv->SendServ(user->fd,"500 "+std::string(user->nick)+" :Only a server may modify the +r channel mode");
				}
			}
			else
			{
				if ((Srv->IsUlined(user->nick)) || (Srv->IsUlined(user->server)) || (!strcmp(user->server,"") || (strchr(user->nick,'.'))))
				{
					log(DEBUG,"Allowing umode +r, server and nick are: '%s','%s'",user->nick,user->server);
					return 1;
				}
				else
				{
					log(DEBUG,"Only a server can set umode +r, server and nick are: '%s','%s'",user->nick,user->server);
					Srv->SendServ(user->fd,"500 "+std::string(user->nick)+" :Only a server may modify the +r user mode");
				}
			}
		}
		else if (modechar == 'R')
		{
			if (type == MT_CHANNEL)
			{
				return 1;
			}
		}
		else if (modechar == 'M')
		{
			if (type == MT_CHANNEL)
			{
				return 1;
			}
		}

		return 0;
	}

	virtual int OnUserPreMessage(userrec* user,void* dest,int target_type, std::string &text)
	{
		if (target_type == TYPE_CHANNEL)
		{
			chanrec* c = (chanrec*)dest;
			if ((c->IsCustomModeSet('M')) && (!strchr(user->modes,'r')))
			{
				if ((Srv->IsUlined(user->nick)) || (Srv->IsUlined(user->server)) || (!strcmp(user->server,"")))
				{
					// user is ulined, can speak regardless
					return 0;
				}
				// user messaging a +M channel and is not registered
				Srv->SendServ(user->fd,"477 "+std::string(user->nick)+" "+std::string(c->name)+" :You need a registered nickname to speak on this channel");
				return 1;
			}
		}
		if (target_type == TYPE_USER)
		{
			userrec* u = (userrec*)dest;
			if ((strchr(u->modes,'R')) && (!strchr(user->modes,'r')))
			{
				if ((Srv->IsUlined(user->nick)) || (Srv->IsUlined(user->server)))
				{
					// user is ulined, can speak regardless
					return 0;
				}
				// user messaging a +R user and is not registered
				Srv->SendServ(user->fd,"477 "+std::string(user->nick)+" "+std::string(u->nick)+" :You need a registered nickname to message this user");
				return 1;
			}
		}
		return 0;
	}
 	
	virtual int OnUserPreNotice(userrec* user,void* dest,int target_type, std::string &text)
	{
		if (target_type == TYPE_CHANNEL)
		{
			chanrec* c = (chanrec*)dest;
			if ((c->IsCustomModeSet('M')) && (!strchr(user->modes,'r')))
			{
				if ((Srv->IsUlined(user->nick)) || (Srv->IsUlined(user->server)))
				{
					// user is ulined, can speak regardless
					return 0;
				}
				// user noticing a +M channel and is not registered
				Srv->SendServ(user->fd,"477 "+std::string(user->nick)+" "+std::string(c->name)+" :You need a registered nickname to speak on this channel");
				return 1;
			}
		}
		if (target_type == TYPE_USER)
		{
			userrec* u = (userrec*)dest;
			if ((strchr(u->modes,'R')) && (!strchr(user->modes,'r')))
			{
				if ((Srv->IsUlined(user->nick)) || (Srv->IsUlined(user->server)))
				{
					// user is ulined, can speak regardless
					return 0;
				}
				// user noticing a +R user and is not registered
				Srv->SendServ(user->fd,"477 "+std::string(user->nick)+" "+std::string(u->nick)+" :You need a registered nickname to message this user");
				return 1;
			}
		}
		return 0;
	}
 	
	virtual int OnUserPreJoin(userrec* user, chanrec* chan, const char* cname)
	{
		if (chan)
		{
			if (chan->IsCustomModeSet('R'))
			{
				if (!strchr(user->modes,'r'))
				{
					if ((Srv->IsUlined(user->nick)) || (Srv->IsUlined(user->server)))
					{
						// user is ulined, won't be stopped from joining
						return 0;
					}
					// joining a +R channel and not identified
					Srv->SendServ(user->fd,"477 "+std::string(user->nick)+" "+std::string(chan->name)+" :You need a registered nickname to join this channel");
					return 1;
				}
			}
		}
		return 0;
	}

	virtual ~ModuleServices()
	{
		delete Srv;
	}
	
	virtual Version GetVersion()
	{
		return Version(1,0,0,0,VF_STATIC|VF_VENDOR);
	}
	
	virtual void OnUserConnect(userrec* user)
	{
	}

};


class ModuleServicesFactory : public ModuleFactory
{
 public:
	ModuleServicesFactory()
	{
	}
	
	~ModuleServicesFactory()
	{
	}
	
	virtual Module * CreateModule()
	{
		return new ModuleServices;
	}
	
};


extern "C" void * init_module( void )
{
	return new ModuleServicesFactory;
}

