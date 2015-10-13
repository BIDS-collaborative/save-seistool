c	$Id: station_hdr.inc,v 1.1 2002/04/05 00:40:25 lombard Exp $
c
c structure for station header
c
      structure /OUT_HDR/
            character*(DH_STATION_LEN) station_id
            character*(DH_LOCATION_LEN) location_id
            character*(DH_CHANNEL_LEN) channel_id
            character*(DH_NETWORK_LEN) network_id
            record /INT_TIME/ begtime
            record /INT_TIME/ endtime
            integer num_pts
            integer sample_rate
            double precision slew
            double precision thresh
            real slat
            real slon
            real sele
		real azi
		real dip
            integer iunit
            real ds
            real gain
            integer npoles
            integer nzeros
            complex poles(60)
            real sens
		real fl
		real fh
            character*32 desc
      end structure
      record /OUT_HDR/ head
