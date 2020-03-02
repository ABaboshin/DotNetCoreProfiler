using System;
using System.Collections.Generic;
using Microsoft.AspNetCore.Mvc;

namespace SampleApp.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    public class ValuesController : ControllerBase
    {
        // GET api/values
        [HttpGet]
        public ActionResult<IEnumerable<string>> Get()
        {
            Test(1);
            Test(1,2);

            SomeOther(1, "x", j: 3);
            SomeOther(2, "y");

            return new string[] { "value1", "value2" };
        }

        void SomeOther(int i, string s, object o = null, int j = 2)
        {
            Console.WriteLine();
        }

        void Test(params object[] list) {
            for (int i = 0; i < list.Length; i++)
            {
                Console.Write(list[i] + " ");
            }
        }

        // GET api/values/5
        [HttpGet("{id}")]
        public ActionResult<string> Oops(int id)
        {
            return NewMethod();
        }

        private static ActionResult<string> NewMethod()
        {
            int i = new Random().Next();
            int j = new Random().Next();
            if (i == j)
            {
                return String.Empty;
            }

            throw new System.Exception();
        }

        // POST api/values
        [HttpPost]
        public void Post([FromBody] string value)
        {
        }

        // PUT api/values/5
        [HttpPut("{id}")]
        public void Put(int id, [FromBody] string value)
        {
        }

        // DELETE api/values/5
        [HttpDelete("{id}")]
        public void Delete(int id)
        {
        }
    }
}
