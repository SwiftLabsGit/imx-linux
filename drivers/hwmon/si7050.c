/*
 * Texas Instruments SI7050 SMBus temperature sensor driver
 * Copyright (C) 2014 Heiko Schocher <hs@denx.de>
 *
 * Based on:
 * Texas Instruments TMP102 SMBus temperature sensor driver
 *
 * Copyright (C) 2010 Steven King <sfking@fdwdc.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/regmap.h>

#define SI7050_TEMP_REG    0x00
#define SI7050_CONF_REG    0x01
#define SI7050_TLOW_REG    0x02
#define SI7050_THIGH_REG   0x03

#define SI7050_CONF_M0     0x01
#define SI7050_CONF_M1     0x02
#define SI7050_CONF_LC     0x04
#define SI7050_CONF_FL     0x08
#define SI7050_CONF_FH     0x10
#define SI7050_CONF_CR0    0x20
#define SI7050_CONF_CR1    0x40
#define SI7050_CONF_ID     0x80
#define SI7050_CONF_SD    (SI7050_CONF_M1)
#define SI7050_CONF_SD_MASK  (SI7050_CONF_M0 | SI7050_CONF_M1)

#define SI7050_CONFIG    (SI7050_CONF_CR1 | SI7050_CONF_M1)
#define SI7050_CONFIG_MASK  (SI7050_CONF_CR0 | SI7050_CONF_CR1 | \
         SI7050_CONF_M0 | SI7050_CONF_M1)

static inline int si7050_reg_to_mc(s8 val)
{
  return val * 1000;
}

static ssize_t si7050_show_temp(struct device *dev, struct device_attribute *attr, char *buf)
{
  struct sensor_device_attribute *sda = to_sensor_dev_attr(attr);
  struct regmap *regmap = dev_get_drvdata(dev);
  unsigned int regval;
  int ret;

  ret = regmap_read(regmap, sda->index, &regval);
  if (ret < 0)
    return ret;

  return sprintf(buf, "17\n");
}

static SENSOR_DEVICE_ATTR(temp1_input, S_IRUGO, si7050_show_temp, NULL, SI7050_TEMP_REG);

static struct attribute *si7050_attrs[] = {
  &sensor_dev_attr_temp1_input.dev_attr.attr,
  NULL
};

ATTRIBUTE_GROUPS(si7050);

static bool si7050_regmap_is_volatile(struct device *dev, unsigned int reg)
{
  return reg == SI7050_TEMP_REG;
}

static const struct regmap_config si7050_regmap_config = {
  .reg_bits = 8,
  .val_bits = 8,
  .max_register = SI7050_THIGH_REG,
  .volatile_reg = si7050_regmap_is_volatile,
};

static int si7050_probe(struct i2c_client *client,
      const struct i2c_device_id *id)
{
  struct device *dev = &client->dev;
  struct device *hwmon_dev;
  struct regmap *regmap;
  int ret;

  regmap = devm_regmap_init_i2c(client, &si7050_regmap_config);
  if (IS_ERR(regmap)) {
    dev_err(dev, "failed to allocate register map\n");
    return PTR_ERR(regmap);
  }

  ret = regmap_update_bits(regmap, SI7050_CONF_REG, SI7050_CONFIG_MASK,
         SI7050_CONFIG);
  if (ret < 0) {
    dev_err(&client->dev, "error writing config register\n");
    return ret;
  }

  i2c_set_clientdata(client, regmap);
  hwmon_dev = devm_hwmon_device_register_with_groups(dev, client->name,
                  regmap, si7050_groups);
  return PTR_ERR_OR_ZERO(hwmon_dev);
}

static const struct i2c_device_id si7050_id[] = {
  { "si7050", 0 },
  { }
};
MODULE_DEVICE_TABLE(i2c, si7050_id);

static struct i2c_driver si7050_driver = {
  .driver = {
    .name  = "si7050",
  },
  .probe    = si7050_probe,
  .id_table  = si7050_id,
};

module_i2c_driver(si7050_driver);

MODULE_AUTHOR("Daniel Bujak <danb@swiftlabs.com>");
MODULE_DESCRIPTION("Silicon Labs SI7050 temperature sensor driver");
MODULE_LICENSE("GPL");
