using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Threading;

[assembly: InternalsVisibleTo("Interception.DeadlockDetection.UnitTests")]
namespace Interception.DeadlockDetection.DataStructures
{
    internal class Graph
    {
        private readonly ConcurrentDictionary<Node, ConcurrentDictionary<Node, Edge>> _info = new ConcurrentDictionary<Node, ConcurrentDictionary<Node, Edge>>();

        public void AddEdge(Node fromNode, Node toNode)
        {
            var add = _info.GetOrAdd(fromNode, new ConcurrentDictionary<Node, Edge>());
            Interlocked.Increment(ref add.GetOrAdd(toNode, new Edge(fromNode, toNode)).Counter);
        }

        public void RemoveEdge(Node fromNode, Node toNode)
        {
            if (!_info.TryGetValue(fromNode, out var value))
            {
                return;
            }

            if (!value.TryGetValue(toNode, out var edge) || Interlocked.Decrement(ref edge.Counter) > 0)
            {
                return;
            }

            value.TryRemove(toNode, out edge);
            if (value.Any())
            {
                return;
            }

            _info.TryRemove(fromNode, out value);
        }

        /// <summary>
        /// find cycles in graph for a given node
        /// </summary>
        /// <param name="node"></param>
        /// <param name="cycles"></param>
        public void FindCycles(Node node, out IList<Edge> cycles)
        {
            cycles = new List<Edge>();
            var index = 0;
            var stack = new Stack<Node>();
            var vindex = new Dictionary<Node, int>();
            var vlowlink = new Dictionary<Node, int>();
            var onStack = new Dictionary<Node, bool>();
            FindCycles(node, ref index, ref stack, ref vindex, ref vlowlink, ref onStack, ref cycles);
        }

        /// <summary>
        /// find all cycles in graph
        /// </summary>
        /// <param name="cycles"></param>
        public void FindCycles(out IList<Edge> cycles)
        {
            cycles = new List<Edge>();
            var index = 0;
            var stack = new Stack<Node>();
            var vindex = new Dictionary<Node, int>();
            var vlowlink = new Dictionary<Node, int>();
            var onStack = new Dictionary<Node, bool>();

            foreach (var node in _info.Keys)
            {
                FindCycles(node, ref index, ref stack, ref vindex, ref vlowlink, ref onStack, ref cycles);
            }
        }

        /// <summary>
        /// Tarjan's strongly connected components algorithm
        /// </summary>
        /// <param name="v"></param>
        /// <param name="index"></param>
        /// <param name="stack"></param>
        /// <param name="vindex"></param>
        /// <param name="vlowlink"></param>
        /// <param name="onStack"></param>
        /// <param name="cycles"></param>
        private void FindCycles(Node v, ref int index, ref Stack<Node> stack, ref Dictionary<Node, int> vindex, ref Dictionary<Node, int> vlowlink, ref Dictionary<Node, bool> onStack, ref IList<Edge> cycles)
        {
            vindex[v] = index;
            vlowlink[v] = index;
            index++;
            stack.Push(v);
            onStack[v] = true;

            if (_info.TryGetValue(v, out var data)) foreach (var item in data)
            {
                if (!vindex.ContainsKey(item.Key))
                {
                    FindCycles(item.Key, ref index, ref stack, ref vindex, ref vlowlink, ref onStack, ref cycles);
                    vlowlink[v] = Math.Min(vlowlink[v], vlowlink[item.Key]);
                }
                else if (onStack[item.Key])
                {
                    vlowlink[v] = Math.Min(vlowlink[v], vindex[item.Key]);
                }
            }

            if (vlowlink[v] == vindex[v])
            {
                while (true)
                {
                    var w = stack.Pop();
                    if (v == w)
                    {
                        break;
                    }

                    cycles.Add(new Edge(w, v));
                }
            }
        }
    }
}
