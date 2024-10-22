log.debug(pv.number)
log.debug(pv.title)
log.debug(pv.very_cool)

log.debug(pv.freight_car_1.cargo)

for k, v in pairs(pv['freight_car_1']) do
  log.debug(k, v)
end
