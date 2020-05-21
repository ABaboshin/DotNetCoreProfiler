using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;

namespace Interception.Core
{
    /// <summary>
    /// base interceptor
    /// </summary>
    public abstract class BaseInterceptor : IInterceptor
    {
        private IMethodFinder _methodFinder = new MethodFinder();

        private object[] _parameters;
        private List<int> _modifiedParameter = new List<int>();

        public object This { get; set; }
        public int MdToken { get; set; }
        public long ModuleVersionPtr { get; set; }
        public object Result { get; set; } = null;
        public Exception Exception { get; set; } = null;

        public object GetParameter(int num)
        {
            return _parameters[num];
        }

        public void ModifyParameter(int num, object value)
        {
            _parameters[num] = value;
            _modifiedParameter.Add(num);
        }

        public bool IsParameterModified(int num)
        {
            return _modifiedParameter.Where(n => n == num).Any();
        }

        public virtual void ExecuteBefore()
        { 
        }

        public virtual void ExecuteAfter()
        { 
        }

        public virtual bool SkipExecution()
        {
            return false;
        }

        protected virtual MethodInfo FindMethod()
        {
            return (MethodInfo)_methodFinder.FindMethod(MdToken, ModuleVersionPtr);
        }

        public void SetParameters(object[] parameters)
        {
            _parameters = parameters;
        }
    }
}
