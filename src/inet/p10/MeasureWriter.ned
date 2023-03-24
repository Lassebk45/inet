package inet.p10;

simple MeasureWriter{
    parameters:
        double writeInterval @mutable = default(30s) @unit(s);
}