#!/bin/sh

# user API key for channel creation
userKey=USER_KEY

# write API key for sensor registration channel
registrationKey=REG_WRITE_KEY

# overwrite the above variables with actual values
source private/config.sh

bold=$(tput bold)
normal=$(tput sgr0)

# get some info for the new channel
echo "\n${bold}Channel title?${normal}"
read channelTitle

echo "${bold}Graph hex value?${normal}"
read hexCode

# spinup new channel
# @see https://www.mathworks.com/help/thingspeak/createchannel.html
createResponse=$(curl -X POST \
    --data-urlencode "api_key=$userKey" \
    --data-urlencode "name=ArduinoAQI / $channelTitle" \
    --data-urlencode "field1=PM 1.0 (µg/m³)" \
    --data-urlencode "field2=PM 2.5 (µg/m³)" \
    --data-urlencode "field3=PM 10.0 (µg/m³)" \
    --data-urlencode "field4=AQI" \
    --data-urlencode "metadata={\"color\": \"$hexCode\"}" \
    https://api.thingspeak.com/channels.json --silent)

channelId=$(echo $createResponse | jq -rc .id)
writeKey=$(echo $createResponse | jq -rc '.api_keys[] | select(.write_flag) | .api_key')

# validate channel create
if [[ $channelId == "null" || -z $writeKey ]]
then
	echo "\n${bold}Channel creation failed! Quitting…${normal}"
	exit 0
fi

echo "\nChannel successfully created!\n"

# add sensor to registration channel
echo "${bold}New sensor MAC address?${normal}"
read macAddress

# @todo validate
curl "https://api.thingspeak.com/update.json?api_key=$registrationKey&field1=$macAddress&field2=$channelId&field3=$writeKey"
echo "\n"
