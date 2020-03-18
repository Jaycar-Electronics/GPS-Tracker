<template>
  <v-card elevation="8" dense>
    <v-card-title> Jaycar GPS Tracker </v-card-title>
    <v-card-text>
      <v-row>
        <v-select
          :items="ports"
          label="Serial Port"
          dense
          outlined
          ref="portSelection"
          v-model="selected"
          @change="changedPort"
        ></v-select>
        <v-btn icon @click="relistPorts">
          <v-icon> mdi-refresh </v-icon>
        </v-btn>
      </v-row>
      <v-select
        v-if="dates.length > 0"
        :items="dates"
        dense
        outlined
        label="Tracking Data"
        v-model="reading"
        @change="changedReading"
      >
      </v-select>
    </v-card-text>
    <v-card-actions>
      <span :class="status.class">
        {{ status.message }}
      </span>
      <v-spacer />
    </v-card-actions>
  </v-card>
</template>
<style scoped></style>
<script>
import SerialPort from 'serialport';
import * as moment from 'moment';

export default {
  name: 'Connector',

  data: () => ({
    selected: '',
    ports: [],
    serial: null,
    parser: null,
    status: {
      message: 'Welcome!',
      class: '',
    },
    reading: '',
    data: [],
    dates: [],
  }),
  methods: {
    changedReading: function() {
      this.serial.write(`READ ${this.reading}`);
    },
    changedPort: function() {
      var hook = this;
      console.log(`Serial port changed to ${this.selected}`);
      this.serial = new SerialPort(this.selected, { baudRate: 9600 }, function(
        error,
      ) {
        if (error == undefined) {
          hook.status.message = 'Connected!';
          hook.status.class = 'green--text';
        } else {
          hook.status.message = error;
          hook.status.class = 'red--text';
        }
      });
      this.serial.pipe(this.parser);
      this.serial.write('LIST'); // test
    },
    relistPorts: function() {
      let hook = this;
      SerialPort.list().then(function(result) {
        hook.ports.splice(0, hook.ports.length);
        result.forEach(function(p) {
          hook.ports.push(p.path);
        });
      });
    },
    processListing: function(listing) {
      this.dates.splice(0, this.dates.length);
      let hook = this;
      listing.split('|').forEach(function(v) {
        hook.dates.push(v);
      });
    },
    processData: function(data) {
      let trip = [];
      let time = '';
      data.split('|').forEach(function(v) {
        //data is in the format of: "time lat,long"
        let data = v.split(' ');

        if (data.length < 2) {
          return; //invalid data, reject
        }
        //for the first entry point, process time:
        if (time == '') {
          time = moment.utc(data[0], 'HHmmss').format();
        }

        let l = data[1].split(',');
        // latitude and longitude are in degree minutes:
        // first have degrees as integers, then add on the minutes as decimals
        let lat = ~~l[0].substring(0, 2) + parseFloat(l[0].substring(2)) / 60;
        let long = ~~l[1].substring(0, 3) + parseFloat(l[1].substring(3)) / 60;

        trip.push(
          //although we didn't record it, in australia, the lat is negative (southern hemi)
          //if you were elsewhere, you'd want to re-do this.
          [-lat, long],
        );
      });

      this.$emit('loadedTrack', trip, time);
    },
    parseData: function(line) {
      //readline parser reads everything as lines; so this should be called per line.
      //and can appear out of order;
      //4 commands: LIST, READ, DELETE, CLEAR
      //3 responses: LIST, DATA, OK
      if (line.startsWith('LIST')) {
        this.processListing(line.substring(5));
      } else if (line.startsWith('DATA')) {
        this.processData(line.substring(5));
        this.status.class = 'green--text';
        this.status.message = 'Loaded Track Data!';
      } else if (line.startsWith('OK')) {
        this.status.class = 'green--text';
        this.status.message = 'Command success!';
      } else if (line.startsWith('Running')) {
      } else {
        this.status.class = 'red--text';
        this.status.message = line;
      }
    },
  },
  mounted() {
    this.relistPorts();

    let hook = this;

    this.parser = new SerialPort.parsers.Readline();
    this.parser.on('data', this.parseData);
  },
};
</script>
