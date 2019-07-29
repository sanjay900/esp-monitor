<?php
if(isset($_POST['submit'])){
	$xml = new SimpleXMLElement("<?xml version=\"1.0\" encoding=\"utf-8\" ?><miles></miles>");

    $xml->addChild('car', $_POST['car']);
    $xml->addChild('bike', $_POST['bike']);

    $asXML = $xml->asXML();
    $file = fopen("testwrite2.xml","w+");
    fwrite($file,$asXML);
    fclose($file); 
    print_r(error_get_last());

    if(file_exists('./testwrite2.xml'))
    {
        $myXML = file_get_contents('./testwrite2.xml');
        $xml = new SimpleXMLElement($myXML);
        $xmlpretty = $xml->asXML();

        // pretty print the XML in browser
        header('content-type: text/xml');
        echo $xmlpretty;
    }
}
?>