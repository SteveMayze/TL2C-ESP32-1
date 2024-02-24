$( document ).ready(function() { 
  $('#zone3-button' ).addClass( 'opac08' ); 
  $('#zone2-button' ).addClass( 'opac08' ); 
  $('#zone1-button' ).addClass( 'opac08' ); 
  $('#zone-test-button' ).addClass( 'opac08' ); 

  $( "#zone3-button" ).mouseout(function() {
    $('#zone3-button').addClass( 'opac08' ).removeClass( 'opac10' );
  });

  $( "#zone3-button" ).mouseover(function() {
     $('#zone3-button').addClass( 'opac10' ).removeClass( 'opac08' );	
  });

  $( "#zone2-button" ).mouseout(function() {
    $('#zone2-button').addClass( 'opac08' ).removeClass( 'opac10' );
  });

  $( "#zone2-button" ).mouseover(function() {
     $('#zone2-button').addClass( 'opac10' ).removeClass( 'opac08' );	
  });

  $( "#zone1-button" ).mouseout(function() {
    $('#zone1-button').addClass( 'opac08' ).removeClass( 'opac10' );
  });

  $( "#zone1-button" ).mouseover(function() {
     $('#zone1-button').addClass( 'opac10' ).removeClass( 'opac08' );	
  });

  $( "#zone-test-button" ).mouseout(function() {
    $('#zone-test-button').addClass( 'opac08' ).removeClass( 'opac10' );
  });

  $( "#zone-test-button" ).mouseover(function() {
     $('#zone-test-button').addClass( 'opac10' ).removeClass( 'opac08' );	
  });

  setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        $('#betriebsZeit').text( this.responseText );
      }
    };
    xhttp.open("GET", "/updat", true);
    xhttp.send();
  }, 1000 ) ;

});
