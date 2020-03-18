<template>
  <v-app>
    <v-container app fluid fill-height style="padding: 0px; z-index: 0;">
      <l-map
        id="map"
        ref="map"
        :zoom="6"
        :center="[-30, 140]"
        :maxZoom="19"
        :minZoom="6"
        :options="{ zoomControl: false }"
        fill-height
      >
        <l-polyline
          :lat-lngs="trackData"
          :options="polyline"
          @click="trackClick"
          ref="polyline"
        >
          <l-popup
            v-if="trackStartTime.length > 0"
            :lat-lng="trackData[0]"
            :content="`Track started at ${trackStartTime}`"
          >
          </l-popup>
        </l-polyline>
        <l-tile-layer :url="url" :attribution="attribution"> </l-tile-layer>
        <l-control position="topleft">
          <connector @loadedTrack="setTrack"></connector>
        </l-control>
        <l-control position="bottomleft">
          Built by Jaycar-Electronics
        </l-control>
      </l-map>
    </v-container>
  </v-app>
</template>
<style scoped>
#map {
  padding: 0;
  margin: 0;
}
</style>
<script>
import { latLng } from 'leaflet';
import { LMap, LTileLayer, LControl, LPolyline, LPopup } from 'vue2-leaflet';
import * as moment from 'moment';

import Connector from './components/Connector';

export default {
  name: 'App',

  components: {
    LMap,
    LTileLayer,
    LControl,
    LPopup,
    LPolyline,
    Connector,
  },

  data: () => ({
    zoom: 13,
    url: 'https://{s}.tile.openstreetmap.fr/hot/{z}/{x}/{y}.png',
    //url: 'https://maps.wikimedia.org/osm-intl/{z}/{x}/{y}.png',
    attribution:
      '&copy; <a href="http://osm.org/copyright">OpenStreetMap</a> contributors',
    mapOptions: {
      zoomSnap: 0.5,
    },
    polyline: {
      color: 'red',
      weight: 3,
      opacity: 1,
      smoothFactor: 1,
    },
    trackData: [],
    trackStartTime: '',
  }),

  methods: {
    trackClick: function() {
      console.log(`track started at ${this.trackStartTime}`);
    },
    setTrack: function(data, time) {
      this.trackData = data;
      if (time.length > 0) {
        this.trackStartTime = moment.utc(time).format('LTS [UTC]');
      } else {
        this.trackStartTime = '';
      }
      this.$nextTick(function() {
        this.$refs.map.mapObject.fitBounds(
          this.$refs.polyline.mapObject._bounds,
        );
      });
    },
  },

  mounted() {
    this.$nextTick(function() {
      this.$refs.map.mapObject.invalidateSize(true);
    });
  },
};
</script>
