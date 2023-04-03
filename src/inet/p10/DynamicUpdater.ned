package inet.p10;

simple DynamicUpdater{
    parameters:
        string updatePath = default("updates.xml");
        double updateInterval = default(0.1s) @unit(s);
}