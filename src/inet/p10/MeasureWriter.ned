package inet.p10;

simple MeasureWriter{
    parameters:
        double writeInterval @mutable = default(5s) @unit(s);
}