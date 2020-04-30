using System;
using System.Collections.Generic;

namespace Interception
{
    public abstract class BaseInterceptor
    {
        protected List<object> _parameters = new List<object>();

        protected object _this;

        protected int _mdToken;

        protected long _moduleVersionPtr;

        public object SetThis(object _this)
        {
            Console.WriteLine($"SetThis {_this}");
            this._this = _this;
            return this;
        }

        public object AddParameter(object value)
        {
            Console.WriteLine($"AddParameter {value}");
            _parameters.Add(value);
            return this;
        }

        public object SetMdToken(int mdToken)
        {
            Console.WriteLine($"SetMdToken {mdToken}");
            _mdToken = mdToken;
            return this;
        }

        public object SetModuleVersionPtr(long moduleVersionPtr)
        {
            Console.WriteLine($"SetModuleVersionPtr {moduleVersionPtr}");
            _moduleVersionPtr = moduleVersionPtr;
            return this;
        }

        //public object Execute()
        //{
        //    return ExecuteInernal();
        //}

        //public void ExecuteVoid()
        //{
        //    ExecuteInernal();
        //}

        //public abstract object ExecuteInernal();
    }
}
