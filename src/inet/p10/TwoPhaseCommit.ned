package inet.p10;

simple TwoPhaseCommit{
    parameters:
        string updatePath = default("2-phase-commit.xml");
        double updateInterval = default(2s) @unit(s);
        double highestLatencyMS @mutable = default(200ms) @unit(ms);
}