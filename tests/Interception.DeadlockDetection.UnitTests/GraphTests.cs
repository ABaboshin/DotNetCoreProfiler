using Interception.DeadlockDetection.DataStructures;
using NUnit.Framework;
using System.Linq;

namespace Interception.DeadlockDetection.UnitTests
{
    public class GraphTests
    {
        [Test]
        public void Find2Edges()
        {
            var graph = new Graph();
            var v1 = new Node("v1", LockType.Monitor);
            var v2 = new Node("v2", LockType.Monitor);
            var v3 = new Node("v3", LockType.Monitor);

            graph.AddEdge(v1, v2);
            graph.AddEdge(v2, v3);
            graph.AddEdge(v3, v1);

            graph.FindCycles(out var result);

            Assert.AreEqual(result.Count(), 2);
        }
    }
}