using System;

namespace Interception.DeadlockDetection.DataStructures
{
    internal class Edge : IEquatable<Edge>
    {
        public Node Prev { get; set; }
        public Node Next { get; set; }
        public volatile int Counter = 0;

        public Edge(Node prev, Node next)
        {
            Prev = prev;
            Next = next;
        }

        public bool Equals(Edge other)
        {
            return Equals(Prev, other.Prev) && Equals(Next, other.Next);
        }

        public override bool Equals(object obj)
        {
            return Equals((Edge)obj);
        }

        public override int GetHashCode()
        {
            return HashCode.Combine(Prev, Next);
        }

        public override string ToString()
        {
            return $"{Prev}->{Next}";
        }
    }
}
