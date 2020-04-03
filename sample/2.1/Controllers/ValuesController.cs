using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;

namespace SampleApp.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    public class ValuesController : ControllerBase
    {
        //public ValuesController()
        //{
        //    try
        //    {

        //    }
        //    catch (Exception ex)
        //    {
        //        Console.WriteLine(ex);
        //    }
        //}

        // GET api/values
        [HttpGet]
        public async Task<ActionResult<IEnumerable<string>>> Get()
        {
            //var test = new TestC();
            //var type = test.GetType();
            //var method = type.GetMethod("Test");
            

            await Program.ATest();
            //await test.Test1Async();
            //var res = await test.Test2Async();
            //Console.WriteLine(res);
            return Ok(AppDomain.CurrentDomain.GetAssemblies().Select(a => a.FullName));
        }

        //// GET api/values/5
        //[HttpGet("{id}")]
        //public ActionResult<string> Get(int id)
        //{
        //    return "value";
        //}

        //// POST api/values
        //[HttpPost]
        //public void Post([FromBody] string value)
        //{
        //}

        //// PUT api/values/5
        //[HttpPut("{id}")]
        //public void Put(int id, [FromBody] string value)
        //{
        //}

        //// DELETE api/values/5
        //[HttpDelete("{id}")]
        //public void Delete(int id)
        //{
        //}
    }
}
