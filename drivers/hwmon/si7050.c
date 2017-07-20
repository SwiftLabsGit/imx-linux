/*
 * PLACEHOLDER
 */

#include <linux/i2c.h>
#include <linux/bcd.h>
#include <linux/rtc.h>
#include <linux/module.h>

#define DRV_VERSION "0.0.1"

#define SI7050_REG_TEMP      0xE3 /* Temperature, Hold Mode */

static struct i2c_driver si7050_driver;

struct si7050 {
  struct rtc_device *rtc;
  int c_polarity;  /* 0: MO_C=1 means 19xx, otherwise MO_C=1 means 20xx */
  int voltage_low; /* indicates if a low_voltage was detected */
};


static int si7050_get_datetime(struct i2c_client *client, char *aTemp)
{
  struct si7050 *si7050 = i2c_get_clientdata(client);
  unsigned char buf[4] = { SI7050_REG_TEMP };
  struct i2c_msg msgs[] = {
    {/* setup read ptr */
      .addr = client->addr,
      .len = 1,
      .buf = buf
    },
    {/* read temperature */
      .addr = client->addr,
      .flags = I2C_M_RD,
      .len = 2,
      .buf = buf
    },
  };

  /* read registers */
  if ((i2c_transfer(client->adapter, msgs, 2)) != 2) {
    dev_err(&client->dev, "%s: read error\n", __func__);
    return -EIO;
  }

  &aTemp = buf[1];

  return 0;
}

static int si7050_read_temp(struct device *dev, char *aTemp)
{
  return si7050_get_datetime(to_i2c_client(dev), tm);
}

static const struct rtc_class_ops si7050_rtc_ops = {
  .read_time  = si7050_read_temp,
  .set_time  = si7050_rtc_set_time
};

static int si7050_probe(struct i2c_client *client,
        const struct i2c_device_id *id)
{
  struct si7050 *si7050;

  dev_dbg(&client->dev, "%s\n", __func__);

  if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
    return -ENODEV;

  si7050 = devm_kzalloc(&client->dev, sizeof(struct si7050),
        GFP_KERNEL);
  if (!si7050)
    return -ENOMEM;

  dev_info(&client->dev, "chip found, driver version " DRV_VERSION "\n");

  i2c_set_clientdata(client, si7050);

  si7050->rtc = devm_rtc_device_register(&client->dev,
        si7050_driver.driver.name,
        &si7050_rtc_ops, THIS_MODULE);

  return PTR_ERR_OR_ZERO(si7050->rtc);
}

static const struct i2c_device_id si7050_id[] = {
  { "si7050", 0 },
  { }
};
MODULE_DEVICE_TABLE(i2c, si7050_id);

#ifdef CONFIG_OF
  static const struct of_device_id si7050_of_match[] = {
    { .compatible = "si,si7050" },
    {}
  };
  MODULE_DEVICE_TABLE(of, si7050_of_match);
#endif

static struct i2c_driver si7050_driver = {
  .driver    = {
    .name  = "si7050",
    .owner  = THIS_MODULE,
    .of_match_table = of_match_ptr(si7050_of_match),
  },
  .probe    = si7050_probe,
  .id_table  = si7050_id,
};

module_i2c_driver(si7050_driver);

MODULE_AUTHOR("DBJ");
MODULE_DESCRIPTION("SI7050 Temperature Sensor Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
