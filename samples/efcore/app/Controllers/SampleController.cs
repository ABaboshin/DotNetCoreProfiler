using Interception.Attributes.Cache;
using Interception.Attributes;
using Microsoft.AspNetCore.Mvc;
using Microsoft.EntityFrameworkCore;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net.Http;
using System.Threading.Tasks;
using System;
using app.Database.Entities;
using app.Database;

namespace app.Controllers
{
  [Route("api/[controller]")]
  [ApiController]
  public class SampleController : ControllerBase
  {
    private readonly MyDbContext _myDbContext;

    public SampleController(MyDbContext myDbContext)
    {
        _myDbContext = myDbContext;
    }

    [HttpGet]
    public async Task<ActionResult<IEnumerable<MyEntity>>> Get()
    {
        _myDbContext.Database.ExecuteSqlCommand("SELECT 1;");
        await _myDbContext.Database.ExecuteSqlCommandAsync("SELECT 2;");

        return await _myDbContext.MyEntities.Where(e => e.Id > 0).ToListAsync();
    }

    [HttpGet("bad")]
    public ActionResult<IEnumerable<BadEntity>> Bad()
    {
        return _myDbContext.BadEntities;
    }
  }
}
