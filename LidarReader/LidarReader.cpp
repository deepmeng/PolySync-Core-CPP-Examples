#include <iostream>
#include "LidarReader.hpp"

Reader::Reader()
    :
      polysync::Node( "LidarReader" )
{
    buildTypeList();
    registerListeners();

    printTypes();
}



void Reader::buildTypeList()
{
    ulong count = 0;
    polysync::message::getCount( reference(), count );

    // register backend to listen for all available types
    for( auto i = 1; i < count+1; ++i )
    {
        int ret = DTC_NONE;
        string name;
        if( ( ret = polysync::message::getNameByType( reference(),
                                                      i,
                                                      name,
                                                      128 ) )
        != DTC_NONE )
        {
            polysync::logMessage( LOG_LEVEL_ERROR,
            { "LidarReader failed to register listener: ",
              to_string( i ) } );
        }
        else
        {
            _typeNameMap.insert( { name, i } );
        }
    }
}



void Reader::registerListeners()
{
    cout << endl << "Registering Callback To: " << endl;
    for( auto type : _typeNameMap )
    {
        if( type.first == "ps_lidar_points_msg" )
        {
            cout << '\t' << type.first << endl;
            int ret = DTC_NONE;
            if( ( ret = polysync::message::registerListener( reference(),
                                                             type.second,
                                                             polysyncCallback,
                                                             this ) )
            != DTC_NONE )
            {
                polysync::logMessage( LOG_LEVEL_ERROR,
                { "Example Reader failed to register listener: ",
                  std::to_string( type.second ) } );
            }
        }
    }
}

void Reader::printTypes()
{
    // print the loaded PolySync Data Model message names and their associated value
    cout << endl << "Existing Data Model Types: " << endl;
    for( auto type : _typeNameMap )
    {
        cout << "\t"
             << "Name: " << type.first
             << '\t'
             << "Value: " << type.second << endl;
    }
    cout << endl;
}



void Reader::printMessage( polysync::Message *message )
{
    // you can filter for specific message types here
    // this example only listens for LiDAR point messages

    cout << endl;
   if( message->messageType() ==
             _typeNameMap.at( "ps_lidar_points_msg" ) )
    {

    // access and filter lidar sensor data
    filterLidarPoints < polysync::LidarPointsMessage >( message );

    }
}



template< typename T >
void Reader::filterLidarPoints( polysync::Message *message )
{

    // Safe cast, if it fails. Our class is not a sub-class os polysync::Message
    if( auto castedMessage = static_cast< T* >( message ) )
    {
        std::vector< ps_lidar_point > lidarPoints = castedMessage->getPoints();

        // iterate through our local vector of ps_lidar_pts
        for( auto point : lidarPoints )
        {
            // intensity : [0:255]
            //      0   : invalid or unknown
            //      1   : lowest intensity
            //      255 : highest intensity

            // ignore points of unknown and not available
            // filter points with intsity lower than an arbitrary number for now
            if( point.intensity >= 50 )
            {
                cout << "Intensity: " << static_cast< short > (point.intensity) << endl;
                // print the xyz position (in reference to the vehicle) of each filtered point
                cout << "Point :" << endl;
                cout << '\t' << "X: " << static_cast< short > (point.position[0]) << endl;
                cout << '\t' << "Y: " << static_cast< short > (point.position[1]) << endl;
                cout << '\t' << "Z: " << static_cast< short > (point.position[2]) << endl;
            }
        }
    }
}



void Reader::execute()
{
    while( 1 )
    {
        // get a message from the queue
        auto message = queuePop();

        // validate message
        if( message )
        {
            printMessage( message.get() );
        }

        // sleep for 1/10 of a second (Default)
        polysync::sleepMicro( _sleepInterval );
    }
}
