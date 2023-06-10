package inet.p10;

simple TwoPhaseCommit{
    parameters:
        string updatePath = default("2-phase-commit.xml");
        string updateTimePath = default("iteration_time.txt");
        double updateInterval @mutable = default(120s) @unit(s);
        double highestLatencyMS @mutable = default(200ms) @unit(ms);
}