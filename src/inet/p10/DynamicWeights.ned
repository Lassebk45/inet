package inet.p10;

simple DynamicWeights{
    parameters:
        string initialPath = default("dynamic_weights-initial.xml");
        string updatePath = default("dynamic_weights.xml");
        double updateInterval @mutable = default(120s) @unit(s);
}