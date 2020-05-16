using System;

namespace Interception.Core
{
    /// <summary>
    /// interceptor
    /// </summary>
    public interface IInterceptor
    {
        /// <summary>
        /// set this of intercepted class
        /// </summary>
        /// <param name="_this"></param>
        /// <returns></returns>
        void SetThis(object _this);

        /// <summary>
        /// get this of intercepted class
        /// </summary>
        /// <returns></returns>
        object GetThis();

        /// <summary>
        /// set argument number
        /// </summary>
        /// <param name="number"></param>
        /// <returns></returns>
        void SetArgumentNumber(int number);

        /// <summary>
        /// add parameter of intercepted method
        /// </summary>
        /// <param name="num"></param>
        /// <param name="value"></param>
        /// <returns></returns>
        void AddParameter(int num, object value);

        /// <summary>
        /// get paraemeter value
        /// </summary>
        /// <param name="num"></param>
        /// <returns></returns>
        object GetParameter(int num);

        /// <summary>
        /// update parameter value
        /// </summary>
        /// <param name="num"></param>
        /// <param name="value"></param>
        void UpdateParameter(int num, object value);

        /// <summary>
        /// set parameters
        /// </summary>
        /// <param name="parameters"></param>
        void SetParameters(object[] parameters);

        /// <summary>
        /// get parameters
        /// </summary>
        /// <returns></returns>
        object[] GetParameters();

        /// <summary>
        /// set metadata token of intercepted method
        /// </summary>
        /// <param name="mdToken"></param>
        /// <returns></returns>
        void SetMdToken(int mdToken);

        /// <summary>
        /// get metadata token of intercepted method
        /// </summary>
        /// <returns></returns>
        int GetMdToken();

        /// <summary>
        /// set pointer to module version of intercepted method
        /// </summary>
        /// <param name="moduleVersionPtr"></param>
        /// <returns></returns>
        void SetModuleVersionPtr(long moduleVersionPtr);

        /// <summary>
        /// get pointer to module version of intercepted method
        /// </summary>
        /// <returns></returns>
        long GetModuleVersionPtr();

        /// <summary>
        /// set key to emit other interceptors
        /// </summary>
        /// <param name="key"></param>
        void SetKey(string key);

        /// <summary>
        /// get key to emit other interceptors
        /// </summary>
        /// <returns></returns>
        string GetKey();

        /// <summary>
        /// execute interceptor
        /// </summary>
        /// <returns></returns>
        object Execute();

        /// <summary>
        /// execute before
        /// </summary>
        void ExecuteBefore();

        /// <summary>
        /// execute after
        /// </summary>
        /// <param name="result"></param>
        /// <param name="exception"></param>
        void ExecuteAfter(object result, Exception exception);

        /// <summary>
        /// execution priority, asc
        /// </summary>
        int Priority { get; }
    }
}
