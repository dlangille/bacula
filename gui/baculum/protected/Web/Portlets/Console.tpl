<script type="text/javascript">
	var show_hide_console = function() {
		$('#<%=$this->ConsoleContainer->ClientID%>').slideToggle({ duration: 200 , done: function() {
			document.getElementById('<%=$this->OutputListing->ClientID%>').scrollTop = document.getElementById('<%=$this->OutputListing->ClientID%>').scrollHeight;
			$('#<%=$this->getPage()->VolumesTools->Tools->ClientID%>').hide();
			document.getElementById('console_launcher').getElementsByTagName('span')[0].innerHTML = (document.getElementById('<%=$this->ConsoleContainer->ClientID%>').style.display == '') ? '<%[ hide console ]%>' : '<%[ show console ]%>';
			$('#<%=$this->CommandLine->ClientID%>').focus();
			window.scrollTo(0, document.body.scrollHeight);
		}});
	};
	$('#console_launcher').on('click', function(){
		show_hide_console();
	});
</script>
<com:TActivePanel ID="ConsoleContainer" DefaultButton="Enter" Style="text-align: left; display: none;">
	<com:TActiveTextBox ID="OutputListing" TextMode="MultiLine" CssClass="console" />
	<com:TActiveTextBox ID="CommandLine" TextMode="SingleLine" CssClass="textbox" Width="760px" Style="margin: 3px 7px; float: left" />
	<com:TActiveButton ID="Enter" Text="<%[ Enter ]%>" OnCommand="sendCommand">
		<prop:ClientSide.OnLoading>
			document.getElementById('<%=$this->CommandLine->ClientID%>').readOnly = true;
		</prop:ClientSide.OnLoading>
		<prop:ClientSide.OnComplete>
			document.getElementById('<%=$this->OutputListing->ClientID%>').scrollTop = document.getElementById('<%=$this->OutputListing->ClientID%>').scrollHeight;
			document.getElementById('<%=$this->CommandLine->ClientID%>').readOnly = false;
			$('#<%=$this->CommandLine->ClientID%>').focus();
		</prop:ClientSide.OnComplete>
	</com:TActiveButton>
	<com:TActiveButton ID="Clear" Text="<%[ Clear ]%>" OnCallback="clearConsole" Style="margin: auto 5px; " />
	<script type="text/javascript">
	$('#<%=$this->CommandLine->ClientID%>').off('keydown');
	$('#<%=$this->CommandLine->ClientID%>').on('keydown', function(e) {
		if (e.keyCode == 27) { // close console on pressed ESC key
			show_hide_console();
		}
	});
	</script>
</com:TActivePanel>
