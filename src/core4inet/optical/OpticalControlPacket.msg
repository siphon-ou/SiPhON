
message OpticalControlPacket
{
    int source;
    int destination;
    int hopCount = 0;
    int type;
    int bufferOccupancy1=-1;
    int bufferOccupancy2=-1;
    int bufferOccupancy3=-1;
    int64_t counter;
}

