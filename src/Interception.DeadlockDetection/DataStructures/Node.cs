using System;

namespace Interception.DeadlockDetection.DataStructures
{
    internal class Node : IEquatable<Node>
    {
        public object MonitorObject { get; set; }
        public LockType LockType { get; set; }

        public Node(object monitorObject, LockType lockType)
        {
            MonitorObject = monitorObject;
            LockType = lockType;
        }

        public bool Equals(Node other)
        {
            return Equals(MonitorObject, other.MonitorObject) && LockType == other.LockType;
        }

        public override bool Equals(object obj)
        {
            return Equals((Node)obj);
        }

        public override int GetHashCode()
        {
            return HashCode.Combine(MonitorObject, LockType);
        }

        public override string ToString()
        {
            return $"{MonitorObject} of type {LockType}";
        }
    }
}
