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
      if(this.readyState == 4 && this.status == 200){
        console.log("onreadystatechange");
        console.log(this.responseText);    
        docj = JSON.parse(this.responseText);
        for( jj in docj){
          console.log(docj[jj].zone)
          zone = docj[jj].zone.id
          state = docj[jj].zone.state

          // redButton
          zz = "#zone"+zone+"-button"
          zoneBtn = $(zz);
          if ( state == "ENABLED" )
            newClass = "greenButton";
          else if (state == "ACTIVE")
            newClass = "redButton"
          else
            newClass = "greyButton"

          classes = zoneBtn.attr("class").split(/\s+/)
          currClass = "greyButton"
          for( cc in classes){
            if (classes[cc].endsWith("Button")){
              currClass = classes[cc];
              break;
            }
            
          }
          if( newClass != currClass)
            zoneBtn.addClass(newClass).removeClass(currClass)
        }      
      }
    };
    xhttp.open("GET", "/state", true);
    xhttp.send();
  }, 2000 ) ;


});

function submitForm(zone, field){
  console.log("TL2C Submit Form");
  console.log(" >>> zone"+zone+"-form, value: "+field.value);
  this.document.getElementById( "zone"+zone+"-update-enable").setAttribute('value', 'false');
  this.document.getElementById( "zone"+zone+"-form").submit();
}
