 <?xml version="1.0"?>
<!DOCTYPE MultiSiteShop SYSTEM "http://www.parc.com/PSS/xml/MultiSiteShop.dtd">
<MultiSiteShop Name="MyMultiSiteShop" Type="" Status="0" Comment="" Creator="PSS" Version="2.0.4">
  <Shop Name="ExampleShop1" File="example_shop1.shp"/>
  <Shop Name="ExampleShop2" File="example_shop2.shp"/>
  <Shop Name="ExampleShop3" File="example_shop3.shp"/>
  <InterShopDelay SourceShop="ExampleShop1" DestinationShop="ExampleShop2" Symmetric="true" Hour="24" Minute="0"/>
  <InterShopDelay SourceShop="ExampleShop2" DestinationShop="ExampleShop3" Symmetric="true" Hour="48" Minute="0"/>
  <InterShopDelay SourceShop="ExampleShop1" DestinationShop="ExampleShop3" Symmetric="false" Hour="48" Minute="0"/>
  <InterShopDelay SourceShop="ExampleShop3" DestinationShop="ExampleShop1" Symmetric="false" Hour="36" Minute="0"/>
</MultiSiteShop>
