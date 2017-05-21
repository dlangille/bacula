<com:TPanel ID="DirectiveOptions" CssClass="directive_setting">
	<fieldset>
		<legend><%[ Options ]%></legend>
		<ul>
			<li rel="show_all_directives"><%[ Show/hide all resource directives ]%></li>
			<!--li rel="show_raw_config"><%[ Show the resource raw config ]%></li>
			<li rel="save_multiple_hosts"><%[ Save the resource on multiple hosts ]%></li>
			<li rel="save_addition_path"><%[ Save the resource to additional path ]%></li>
			<li rel="download_resource_config"><%[ Download the resource config ]%></li-->
		</ul>
	</fieldset>
</com:TPanel>
<com:TCallback ID="DirectiveOptionCall" OnCallback="setOption">
	<prop:ClientSide.OnComplete>
	</prop:ClientSide.OnComplete>
</com:TCallback>
<script type="text/javascript">
	var BaculaConfigOptions = new BaculaConfigOptionsClass({
		options_id: '<%=$this->DirectiveOptions->ClientID%>',
		action_obj: <%=$this->DirectiveOptionCall->ActiveControl->Javascript%>
	});
</script>
