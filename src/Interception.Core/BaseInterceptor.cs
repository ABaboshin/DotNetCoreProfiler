using Interception.Attributes;
using Interception.Core.Extensions;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Reflection.Emit;
using System.Threading.Tasks;

namespace Interception.Core
{
    /// <summary>
    /// base interceptor
    /// </summary>
    public abstract class BaseInterceptor : IInterceptor
    {
        private IMethodFinder _methodFinder = new MethodFinder();

        private object[] _parameters;

        public object This { get; set; }
        public int MdToken { get; set; }
        public long ModuleVersionPtr { get; set; }
        public object Result { get; set; } = null;
        public Exception Exception { get; set; } = null;

        public void AddParameter(int num, object value)
        {
            //Console.WriteLine($"AddParameter {num} {value}");
            _parameters[num] = value;
        }

        public object GetParameter(int num)
        {
            return _parameters[num];
        }

        public void UpdateParameter(int num, object value)
        {
            _parameters[num] = value;
        }

        public void SetArgumentCount(int number)
        {
            //Console.WriteLine($"SetArgumentNumber {number}");
            _parameters = new object[number];
        }

        public object Execute()
        {
            Validate();
            var method = FindMethod();
            var isAsync = method.IsReturnTypeTask();

            Console.WriteLine($"Execute {method.Name}");

            if (!isAsync)
            {
                return ExecuteSyncInternal();
            }
            else if (!method.IsReturnTypeTaskWithResult())
            {
                return ExecuteAsyncInternal();
            }
            else
            {
                return ExecuteAsyncWithResultInternal();
            }
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

        private List<IInterceptor> _childs = new List<IInterceptor>();
        public void AddChild(IInterceptor interceptor)
        {
            Console.WriteLine($"AddChild {interceptor.GetType().FullName}");
            _childs.Add(interceptor);
        }

        #region internal

        private void ExecuteBeforeInternal()
        {
            ExecuteBefore();
            foreach (var item in _childs)
            {
                item.ExecuteBefore();
            }
        }

        private void ExecuteAfterInternal()
        {
            ExecuteAfter();
            foreach (var item in _childs)
            {
                item.ExecuteAfter();
            }
        }

        private bool SkipExecutionInternal()
        {
            if (SkipExecution())
            {
                return true;
            }

            var skip = false;
            foreach (var item in _childs)
            {
                if (item.SkipExecution())
                {
                    skip = true;
                    Result = item.Result;
                }
            }

            return skip;
        }

        private object ExecuteSyncInternal()
        {
            var method = FindMethod();

            try
            {
                ExecuteBeforeInternal();
                if (!SkipExecutionInternal())
                {
                    Result = method.Invoke(This, _parameters);
                }
                
                ExecuteAfterInternal();
                return Result;
            }
            catch (Exception ex)
            {
                Exception = ex;
                ExecuteAfterInternal();
                throw;
            }
        }

        private async Task ExecuteAsyncInternal()
        {
            var method = FindMethod();

            try
            {
                ExecuteBeforeInternal();
                if (!SkipExecutionInternal())
                {
                    var result = (Task)method.Invoke(This, _parameters);
                    await result;
                    Result = result;
                }

                ExecuteAfterInternal();
            }
            catch (Exception ex)
            {
                Exception = ex;
                ExecuteAfterInternal();
                throw;
            }
        }

        private async Task<T> ExecuteWithMetrics<T>(Delegate funcDelegate)
        {
            try
            {
                ExecuteBeforeInternal();
                if (!SkipExecutionInternal())
                {
                    var func = (Func<Task<T>>)funcDelegate;
                    Result = await func.Invoke();
                }

                ExecuteAfterInternal();
                return (T)Result;
            }
            catch (Exception ex)
            {
                Exception = ex;
                ExecuteAfterInternal();
                throw;
            }
        }

        internal static readonly ModuleBuilder Module;

        static BaseInterceptor()
        {
            var asm = AssemblyBuilder.DefineDynamicAssembly(new AssemblyName("Interception"), AssemblyBuilderAccess.Run);
            Module = asm.DefineDynamicModule("DynamicModule");
        }

        private object ExecuteAsyncWithResultInternal()
        {
            var method = FindMethod();

            var underilyingType = ((TypeInfo)method.ReturnType).GenericTypeArguments[0];

            var createExecutionDelegateGeneric = typeof(BaseInterceptor).GetMethods(BindingFlags.Instance | BindingFlags.NonPublic).Where(m => m.Name == nameof(CreateExecutionDelegate)).First();
            var createExecutionDelegate = createExecutionDelegateGeneric.MakeGenericMethod(underilyingType);
            var executionDelegate = createExecutionDelegate.Invoke(this, new object[] { });

            var executeWithMetricsGeneric = typeof(BaseInterceptor).GetMethods(BindingFlags.Instance | BindingFlags.NonPublic).Where(m => m.Name == nameof(ExecuteWithMetrics)).First();
            var executeWithMetrics = executeWithMetricsGeneric.MakeGenericMethod(underilyingType);

            return executeWithMetrics.Invoke(this, new object[] { executionDelegate });
        }

        private Delegate CreateExecutionDelegate<T>()
        {
            var method = FindMethod();

            Func<Task<T>> func = () => {
                return (Task<T>)method.Invoke(This, _parameters);
            };

            return func;
        }

        protected void Validate()
        {
            var method = FindMethod();
            foreach (var p in method.GetParameters())
            {
                var validationAttributes = p.GetCustomAttributes<ParameterValidationAttribute>();

                foreach (var validationAttribute in validationAttributes)
                {
                    validationAttribute.Validate(_parameters[p.Position]);
                }
            }
        }
        #endregion
    }
}
