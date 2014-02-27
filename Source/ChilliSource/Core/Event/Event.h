/*
 *  Event.h
 *  ChilliSource
 *
 *  Created by Scott Downie on 27/02/2014.
 *  Copyright 2014 Tag Games. All rights reserved.
 *
 */

#ifndef _CHILLISOURCE_CORE_EVENT_EVENT_H_
#define _CHILLISOURCE_CORE_EVENT_EVENT_H_

#include <ChilliSource/ChilliSource.h>
#include <ChilliSource/Core/Event/Connection.h>
#include <ChilliSource/Core/Event/IConnectableEvent.h>
#include <ChilliSource/Core/Event/IDisconnectableEvent.h>

#include <vector>

namespace ChilliSource
{
    namespace Core
    {
        typedef std::shared_ptr<Connection> ConnectionSPtr;
        typedef std::unique_ptr<Connection> ConnectionUPtr;
        typedef std::weak_ptr<Connection> ConnectionWPtr;
        
        //-----------------------------------------------------------------
        /// An Event represents an object which can have multiple listeners
        /// (connections) with the given DelegateType.
        /// Objects should though expose the IConnectableEvent interface
        /// to prevent other objects invoking the event.
        ///
        /// @author S Downie
        //-----------------------------------------------------------------
        template <typename TDelegateType> class Event final : public IConnectableEvent<TDelegateType>, public IDisconnectableEvent
        {
        public:
            //-------------------------------------------------------------
            /// Constructor
            ///
            /// @author S Downie
            //-------------------------------------------------------------
            Event(){}
            //-------------------------------------------------------------
            /// No copying of events is allowed. If you wish to make a
            /// shallow copy of an event then hold a pointer to it.
            ///
            /// @author S Downie
            //-------------------------------------------------------------
            Event(const Event&) = delete;
            Event& operator= (const Event&) = delete;
            
            //-------------------------------------------------------------
            /// Opens a new connection to the event. While this connection
            /// remains in scope the delegate will be notified of events
            ///
            /// @author S Downie
            ///
            /// @param Delegate to notify
            ///
            /// @return Scoped connection
            //-------------------------------------------------------------
            ConnectionUPtr OpenConnection(const TDelegateType& in_delegate) override
            {
                ConnectionUPtr connection(new Connection());
                connection->SetOwningEvent(this);
            
                ConnectionDesc desc;
                desc.m_isOpen = true;
                desc.m_delegate = in_delegate;
                desc.m_connection = connection.get();
                m_connections.push_back(desc);
            
                return connection;
            }
            //-------------------------------------------------------------
            /// Close connection to the event. The connection will
            /// no longer be notified of the event
            ///
            /// @author S Downie
            ///
            /// @param Connection to close
            //-------------------------------------------------------------
            void CloseConnection(Connection* in_connection) override
            {
                for(u32 i=0; i<m_connections.size(); ++i)
                {
                    ConnectionDesc& desc = m_connections[i];
                    if(desc.m_connection == in_connection)
                    {
                        if(m_isNotifying == false)
                        {
                            m_connections.erase(m_connections.begin() + i);
                        }
                        else
                        {
                            desc.m_isOpen = false;
                        }
                        
                        return;
                    }
                }
            }
            //-------------------------------------------------------------
            /// Notify connections that the event has occurred
            ///
            /// @author S Downie
            ///
            /// @param Arguments to pass to the connection delegate
            //-------------------------------------------------------------
            template <typename... TArgTypes> void NotifyConnections(TArgTypes... in_args)
            {
                m_isNotifying = true;
                
                //Take a snapshot of the number of delegates so any new ones added
                //during the notify loop aren't notified themseleves.
                u32 numConnections = m_connections.size();
                for(u32 i=0; i<numConnections; ++i)
                {
                    if(m_connections[i].m_isOpen == true)
                    {
                        m_connections[i].m_delegate(in_args...);
                    }
                }
                
                m_isNotifying = false;
                
                RemoveClosedConnections();
            }
    
        private:
    
            //-------------------------------------------------------------------------
            /// Remove from the list any connections that have been flagged as closed
            ///
            /// @author S Downie
            //-------------------------------------------------------------------------
            void RemoveClosedConnections()
            {
                for(typename ConnectionList::iterator it = m_connections.begin(); it != m_connections.end(); )
                {
                    if(it->m_isOpen == false)
                    {
                        it = m_connections.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
            }
    
        private:
                                    
            struct ConnectionDesc
            {
                TDelegateType m_delegate;
                Connection* m_connection = nullptr;
                bool m_isOpen = false;
            };
    
            typedef std::vector<ConnectionDesc> ConnectionList;
            ConnectionList m_connections;
        
            bool m_isNotifying = false;
        };
    }
}

#endif