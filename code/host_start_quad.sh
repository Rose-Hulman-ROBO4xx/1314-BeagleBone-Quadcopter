./joystick.cal
#ssh -i quad_key root@10.10.1.2 "~/beagle_start_quad.sh"
node joystick.js | tee command_history | netcat $1 1234

