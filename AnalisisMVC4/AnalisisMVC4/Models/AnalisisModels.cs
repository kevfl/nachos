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
    public class UsersContext : DbContext
    {
        public UsersContext()
            : base("Cnn_DB")
        {
        }

        public DbSet<TB_ROUTE_ANALSYS> TB_ROUTE_ANALSYS { get; set; }
        public DbSet<TB_RUNS> TB_RUNS { get; set; }
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
        public Int64 RUN_ID { get; set; }
        public Int64 RUN_R_ANALYSIS_ID { get; set; }
        public int RUN_ROUTE { get; set; }
        public int RUN_ITINERARY { get; set; }
        public DateTime RUN_DATE { get; set; }
        public int RUN_PAX_IN { get; set; }
        public int RUN_PAX_OUT { get; set; }
        public decimal RUN_AMOUNT { get; set; }
        public DateTime RUN_START_TIME { get; set; }
        public DateTime RUN_END_TIME { get; set; }
        public Byte RUN_DIRECTION { get; set; }
    }



    /*Tabla de roles*/
    [Table("webpages_Membership")]
    public class webpages_Membership
    {
        [Key]
        [DatabaseGeneratedAttribute(DatabaseGeneratedOption.Identity)]
        public int RoleId { get; set; }
        public string RoleName { get; set; }
    }

    /*Tablas de roles de usuario*/
    [Table("webpages_UsersInRoles")]
    public class webpages_UsersInRoles
    {
        [Key]
        [DatabaseGeneratedAttribute(DatabaseGeneratedOption.Identity)]
        public int UserId { get; set; }
        public int RoleId { get; set; }
    }









    public class RegisterExternalLoginModel
    {
        [Required]
        [Display(Name = "Nombre de usuario")]
        public string UserName { get; set; }

        public string ExternalLoginData { get; set; }
    }

    public class LocalPasswordModel
    {
        [Required]
        [DataType(DataType.Password)]
        [Display(Name = "Contraseña actual")]
        public string OldPassword { get; set; }

        [Required]
        [StringLength(100, ErrorMessage = "La {0} debe contener al menos {2} carácteres.", MinimumLength = 6)]
        [DataType(DataType.Password)]
        [Display(Name = "Nueva Contraseña")]
        public string NewPassword { get; set; }

        [DataType(DataType.Password)]
        [Display(Name = "Confirmar nueva contraseña")]
        [Compare("NewPassword", ErrorMessage = "La nueva contraseña y la contraseña de confirmacion difieren.")]
        public string ConfirmPassword { get; set; }
    }

    public class LoginModel
    {
        [Required]
        [Display(Name = "Nombre de usuario")]
        public string UserName { get; set; }

        [Required]
        [DataType(DataType.Password)]
        [Display(Name = "Contraseña")]
        public string Password { get; set; }

        [Display(Name = "¿Recordar nombre de usuario?")]
        public bool RememberMe { get; set; }
    }

    public class AddUserModel
    {
        [Required]
        [Display(Name = "Nombre de usuario")]
        public string UserName { get; set; }

        [Required]
        [StringLength(100, ErrorMessage = "La contraseña{0} debe ser de almenos {2} caracteres de extención.", MinimumLength = 6)]
        [DataType(DataType.Password)]
        [Display(Name = "Password")]
        public string Password { get; set; }

        [DataType(DataType.Password)]
        [Display(Name = "Confirm password")]
        [Compare("Password", ErrorMessage = "The password and confirmation password do not match.")]
        public string ConfirmPassword { get; set; }

        [Required]
        [Display(Name = "Rol de usuario")]
        public string Role { get; set; }
    }

    public class ExternalLogin
    {
        public string Provider { get; set; }
        public string ProviderDisplayName { get; set; }
        public string ProviderUserId { get; set; }
    }
}
