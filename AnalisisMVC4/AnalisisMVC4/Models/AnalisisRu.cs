using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;

namespace AnalisisMVC4.Models
{
    public class AnalisisRu
    {
    }
}
using System;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations;
using System.ComponentModel.DataAnnotations.Schema;
using System.Data.Entity;
using System.Globalization;
using System.Web.Mvc;
using System.Web.Security;

namespace AnalisisMVC4.Models
{
    /*
        Base de Datos que moldea.
     */
    public class Cnn_DB : DbContext
    {
        public Cnn_DB()
            : base("Cnn_DB")
        {
        }

        public DbSet<TB_ROUTE_ANALSYS> TB_ROUTE_ANALSYS { get; set; }
        public DbSet<TB_RUNS> TB_RUNS { get; set; }
        public DbSet<TB_DRIVER> TB_DRIVER { get; set; }
        public DbSet<TB_COMPANY> TB_COMPANY { get; set; }
        public DbSet<TB_RECORD> TB_RECORD { get; set; }
        
    }
    /*
        Tablas que moldea
     */
    /*Tabla de ANALISIS DE RUTA*/
    [Table("TB_ROUTE_ANALSYS")]
    public class TB_ROUTE_ANALSYS
    {
        public Int64    RA_ID { get; set; }
        public int      RA_ID_VEHICLE { get; set; }
        public DateTime RA_START_DATETIME { get; set; }
        public DateTime RA_END_DATETIME { get; set; }
        public int      RA_ABSOLUT_COUNT { get; set; }
        public int      RA_ID_DRIVER { get; set; }
        public double   RA_RUNS_COUNT { get; set; }
        public decimal  RA_AMOUNT_REPORT { get; set; }
        public decimal  RA_AMOUNT_CALCULATE { get; set; }
        public int      RA_ID_COMPANY { get; set; }
        public decimal  RA_LACK_MONEY { get; set; }
        public int      RA_PAX_IN { get; set; }
        public int      RA_PAX_OUT { get; set; }
        public int      RA_PAX_OLD_COUNT { get; set; }
        public decimal  RA_PAX_OLD_AMOUNT { get; set; }
        public int      RA_USER { get; set; }
        public DateTime RA_TIME_STAMP { get; set; }
    }




    /*Tabla de corridas*/
    [Table("TB_RUNS")]
    public class TB_RUNS
    {
        public Int64    RUN_ID { get; set; }
        public Int64    RUN_R_ANALYSIS_ID { get; set; }
        public int      RUN_ROUTE { get; set; }
        public int      RUN_ITINERARY { get; set; }
        public DateTime RUN_DATE { get; set; }
        public int      RUN_PAX_IN { get; set; }
        public int      RUN_PAX_OUT { get; set; }
        public decimal  RUN_AMOUNT { get; set; }
        public DateTime RUN_START_TIME { get; set; }
        public DateTime RUN_END_TIME { get; set; }
        public Byte     RUN_DIRECTION { get; set; }
    }


    /*Tabla de conductores*/
    [Table("TB_DRIVER")]
    public class TB_DRIVER
    {
        public int      DR_ID { get; set; }
	    public String   DR_FIRSTNAME { get; set; }
	    public String   DR_LASTNAME { get; set; }
	    public String   DR_ID_NUMBER { get; set; }
	    public int      DR_ID_COMPANY { get; set; }
	    public String   DR_LICENSE_NUMBER { get; set; }
	    public DateTime DR_EXP_DATE_LICENSE { get; set; }
	    public Boolean  DR_ACTIVE { get; set; }
	    public String   DR_PLANILLA_ID { get; set; }
    }

    /*Tabla de compañias*/
    [Table("TB_COMPANY")]
    public class TB_COMPANY
    {
        public int      CO_ID { get; set; }
        public String   CO_NAME { get; set; }
        public String   CO_BUSINESS_NAME { get; set; }
        public String   CO_ID_NUMBER { get; set; }
        public String   CO_DETAIL { get; set; }
    }


    /*Tabla de records*/
    [Table("TB_RECORD")]
    public class TB_RECORD
    {
        public Int64    REC_ID { get; set; }
        public int      REC_IDVEHICLE { get; set; }
        public DateTime REC_TIMESTAMP { get; set; }
        public int      REC_IN_COUNT { get; set; }
        public int      REC_OUT_COUNT { get; set; }
        public double   REC_LATITUDE { get; set; }
        public double   REC_LONGITUDE { get; set; }
        public double   REC_ALTITUDE { get; set; }
        public int      REC_SPEED { get; set; }
        public String   REC_DOOR { get; set; }
        public double   REC_ODOMETER { get; set; }
        public int      REC_COMPANY { get; set; }
        public String   REC_CAN_BUS_INFO { get; set; }
        public int      REC_CAN_BUS_CODE { get; set; }
        public double   REC_ACCELEROMETER { get; set; }
        public String   REC_GPS_STATUS { get; set; }
        public String   REC_PCN_STATUS { get; set; }
        public Byte     REC_DIRECTION { get; set; }
        public Boolean  REC_ANALYZED { get; set; }
        public String   REC_ANALYSIS_TAG { get; set; }
        public Int64    REC_RUN_UNIQUE_CODE { get; set; }
        public Int64    REC_ROUTE_ANALYSIS_ID { get; set; }
        public int      REC_IN_COUNT_NET { get; set; }
        public int      REC_OUT_COUNT_NET { get; set; }
        public DateTime REC_TRANSFER_TIMESTAMP { get; set; }
    }


    public class IndexModel
    {
        
    }

    public class ExternalLogin
    {
        public string Provider { get; set; }
        public string ProviderDisplayName { get; set; }
        public string ProviderUserId { get; set; }
    }
}
