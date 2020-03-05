using System.Threading;
using System.Threading.Tasks;

namespace line_sensor.data_collector.logic
{
    public interface IDataCollector
    {
        Task CollectData(uint dataCollectionPositionDelta, int numberOfSamples, string resultFilePathPrefix, CancellationToken token);
    }
}
