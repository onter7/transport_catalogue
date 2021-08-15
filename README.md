# Transport Catalogue
A program for building a bus traffic scheme and finding optimal routes 

Supports:
- graphical output
- search for routes
- time calculation

## Description
The program is divided into 2 stages. The stage is specified using the command line argument: 
- **make_base** for building a database of bus stops and routes and serializing it into a file using Protobuf
- **process_requests** for processing of various requests - getting information about a bus stop, bus route, finding the shortest path between stops, building a map of bus routes

Requests are transmitted via standard I/O in JSON format. The map is built in SVG format.

## Build
The project supports building using CMake. 

External dependencies: 
- Protobuf - specify the path to Protobuf as follows ```$cmake <path_to_transport_catalogue> -DCMAKE_PREFIX_PATH=<path_to_protobuf>```

## Example

### Make Base
```
{
    "serialization_settings": {
        "file": "transport_catalogue.db"
    },
    "routing_settings": {
        "bus_wait_time": 2,
        "bus_velocity": 30
    },
    "render_settings": {
        "width": 1200,
        "height": 500,
        "padding": 50,
        "stop_radius": 5,
        "line_width": 14,
        "bus_label_font_size": 20,
        "bus_label_offset": [
            7,
            15
        ],
        "stop_label_font_size": 18,
        "stop_label_offset": [
            7,
            -3
        ],
        "underlayer_color": [
            255,
            255,
            255,
            0.85
        ],
        "underlayer_width": 3,
        "color_palette": [
            "green",
            [
                255,
                160,
                0
            ],
            "red"
        ]
    },
    "base_requests": [
        {
            "is_roundtrip": true,
            "name": "297",
            "stops": [
                "Biryulyovo Zapadnoye",
                "Biryulyovo Tovarnaya",
                "Universam",
                "Biryulyovo Zapadnoye"
            ],
            "type": "Bus"
        },
        {
            "is_roundtrip": false,
            "name": "635",
            "stops": [
                "Biryulyovo Tovarnaya",
                "Universam",
                "Prazhskaya"
            ],
            "type": "Bus"
        },
        {
            "latitude": 55.574371,
            "longitude": 37.6517,
            "name": "Biryulyovo Zapadnoye",
            "road_distances": {
                "Biryulyovo Tovarnaya": 2600
            },
            "type": "Stop"
        },
        {
            "latitude": 55.587655,
            "longitude": 37.645687,
            "name": "Universam",
            "road_distances": {
                "Biryulyovo Tovarnaya": 1380,
                "Biryulyovo Zapadnoye": 2500,
                "Prazhskaya": 4650
            },
            "type": "Stop"
        },
        {
            "latitude": 55.592028,
            "longitude": 37.653656,
            "name": "Biryulyovo Tovarnaya",
            "road_distances": {
                "Universam": 890
            },
            "type": "Stop"
        },
        {
            "latitude": 55.611717,
            "longitude": 37.603938,
            "name": "Prazhskaya",
            "road_distances": {},
            "type": "Stop"
        }
    ]
}
```
### Process Requests
```
{
    "serialization_settings": {
        "file": "transport_catalogue.db"
    },
    "stat_requests": [
        {
            "id": 1,
            "name": "297",
            "type": "Bus"
        },
        {
            "id": 2,
            "name": "635",
            "type": "Bus"
        },
        {
            "id": 3,
            "name": "Universam",
            "type": "Stop"
        },
        {
            "from": "Biryulyovo Zapadnoye",
            "id": 4,
            "to": "Universam",
            "type": "Route"
        },
        {
            "from": "Biryulyovo Zapadnoye",
            "id": 5,
            "to": "Prazhskaya",
            "type": "Route"
        },
        {
            "id": 6,
            "type": "Map"
        }
    ]
}
```
### Response
```
[
    {
        "curvature": 1.42963,
        "request_id": 1,
        "route_length": 5990,
        "stop_count": 4,
        "unique_stop_count": 3
    },
    {
        "curvature": 1.30156,
        "request_id": 2,
        "route_length": 11570,
        "stop_count": 5,
        "unique_stop_count": 3
    },
    {
        "buses": [
            "297",
            "635"
        ],
        "request_id": 3
    },
    {
        "items": [
            {
                "stop_name": "Biryulyovo Zapadnoye",
                "time": 6,
                "type": "Wait"
            },
            {
                "bus": "297",
                "span_count": 2,
                "time": 5.235,
                "type": "Bus"
            }
        ],
        "request_id": 4,
        "total_time": 11.235
    },
    {
        "items": [
            {
                "stop_name": "Biryulyovo Zapadnoye",
                "time": 6,
                "type": "Wait"
            },
            {
                "bus": "297",
                "span_count": 2,
                "time": 5.235,
                "type": "Bus"
            },
            {
                "stop_name": "Universam",
                "time": 6,
                "type": "Wait"
            },
            {
                "bus": "635",
                "span_count": 1,
                "time": 6.975,
                "type": "Bus"
            }
        ],
        "request_id": 5,
        "total_time": 24.21
    },    
    {
        "map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\"> ... </svg>",
        "request_id": 6
    }
]
```
