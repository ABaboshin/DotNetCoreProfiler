using System.Threading.Tasks;

namespace SampleApp
{
    public interface ITestC
    {
        string Test();
        Task Test1Async();
        Task<string> Test2Async();
    }
}