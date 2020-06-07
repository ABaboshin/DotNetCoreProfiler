using Interception.Core.Extensions;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Reflection.Emit;
using System.Threading.Tasks;

namespace Interception.Core
{
    public class ComposedInterceptor : IComposedInterceptor
    {
        private List<IInterceptor> _childs = new List<IInterceptor>();
        private object[] _parameters;

        public object This { get; set; }
        public int MdToken { get; set; }
        public long ModuleVersionPtr { get; set; }
        public string MethodName { get; set; }
        public IMethodFinder MethodFinder { get; set; }

        public void AddChild(IInterceptor interceptor)
        {
            _childs.Add(interceptor);
        }

        public void AddParameter(int num, object value)
        {
            _parameters[num] = value;
        }

        public void SetArgumentCount(int number)
        {
            _parameters = new object[number];
        }

        public object GetParameter(int num)
        {
            return _parameters[num];
        }

        private MethodInfo FindMethod()
        {
            if (MethodFinder is null)
            {
                MethodFinder = new DefaultMethodFinder();
            }

            return MethodFinder.FindMethod(MdToken, ModuleVersionPtr, This, _parameters);
        }

        public object Execute()
        {
            if (!_childs.Any(c => c.GetType() == typeof(ValidationInterceptor)))
            {
                _childs.Add(new ValidationInterceptor());
            }

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

        #region internal

        private void ExecuteBefore()
        {
            foreach (var item in _childs.OrderBy(c => c.Priority))
            {
                item.This = This;
                item.Method = FindMethod();
                item.SetParameters(_parameters);
                item.ExecuteBefore();
                for (int i = 0; i < _parameters.Length; i++)
                {
                    if (item.IsParameterModified(i))
                    {
                        _parameters[i] = item.GetParameter(i);
                    }
                }
            }
        }

        private void ExecuteAfter()
        {
            foreach (var item in _childs.OrderByDescending(c => c.Priority))
            {
                item.SetParameters(_parameters);
                item.Exception = _exception;
                item.Result = _result;
                item.ExecuteAfter();
            }
        }

        private object _result = null;
        private Exception _exception = null;

        private bool SkipExecution()
        {
            var skip = false;
            foreach (var item in _childs)
            {
                if (item.SkipExecution())
                {
                    skip = true;
                    _result = item.Result;
                }
            }

            return skip;
        }

        private object ExecuteSyncInternal()
        {
            var method = FindMethod();

            try
            {
                ExecuteBefore();
                if (!SkipExecution())
                {
                    _result = method.Invoke(This, _parameters);
                }

                ExecuteAfter();
                return _result;
            }
            catch (Exception ex)
            {
                _exception = ex;
                ExecuteAfter();
                throw;
            }
        }

        private async Task ExecuteAsyncInternal()
        {
            var method = FindMethod();

            try
            {
                ExecuteBefore();
                if (!SkipExecution())
                {
                    var result = (Task)method.Invoke(This, _parameters);
                    await result;
                    _result = result;
                }

                ExecuteAfter();
            }
            catch (Exception ex)
            {
                _exception = ex;
                ExecuteAfter();
                throw;
            }
        }

        private async Task<T> ExecuteWithMetrics<T>(Delegate funcDelegate)
        {
            try
            {
                ExecuteBefore();
                if (!SkipExecution())
                {
                    var func = (Func<Task<T>>)funcDelegate;
                    _result = await func.Invoke();
                }

                ExecuteAfter();
                return (T)_result;
            }
            catch (Exception ex)
            {
                _exception = ex;
                ExecuteAfter();
                throw;
            }
        }

        internal static readonly ModuleBuilder Module;

        static ComposedInterceptor()
        {
            var asm = AssemblyBuilder.DefineDynamicAssembly(new AssemblyName("Interception"), AssemblyBuilderAccess.Run);
            Module = asm.DefineDynamicModule("DynamicModule");
        }

        private object ExecuteAsyncWithResultInternal()
        {
            var method = FindMethod();

            var underilyingType = ((TypeInfo)method.ReturnType).GenericTypeArguments[0];

            var createExecutionDelegateGeneric = typeof(ComposedInterceptor).GetMethods(BindingFlags.Instance | BindingFlags.NonPublic).Where(m => m.Name == nameof(CreateExecutionDelegate)).First();
            var createExecutionDelegate = createExecutionDelegateGeneric.MakeGenericMethod(underilyingType);
            var executionDelegate = createExecutionDelegate.Invoke(this, new object[] { });

            var executeWithMetricsGeneric = typeof(ComposedInterceptor).GetMethods(BindingFlags.Instance | BindingFlags.NonPublic).Where(m => m.Name == nameof(ExecuteWithMetrics)).First();
            var executeWithMetrics = executeWithMetricsGeneric.MakeGenericMethod(underilyingType);

            return executeWithMetrics.Invoke(this, new object[] { executionDelegate });
        }

        private Delegate CreateExecutionDelegate<T>()
        {
            var method = FindMethod();

            Func<Task<T>> func = () =>
            {
                return (Task<T>)method.Invoke(This, _parameters);
            };

            return func;
        }
        #endregion
    }
}
