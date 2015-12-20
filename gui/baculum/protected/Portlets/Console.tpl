<script type="text/javascript">
	var show_hide_console = function() {
		Effect.toggle('<%=$this->ConsoleContainer->ClientID%>', 'slide', { duration: 0.2 , afterFinish: function() {
			$('<%=$this->OutputListing->ClientID%>').scrollTop = $('<%=$this->OutputListing->ClientID%>').scrollHeight;
			$('<%=$this->getPage()->VolumesTools->Tools->ClientID%>').hide();
			$('console_launcher').down('span').innerHTML = ($('<%=$this->ConsoleContainer->ClientID%>').getStyle('display') == 'block') ? '<%[ hide console ]%>' : '<%[ show console ]%>';
			$('<%=$this->CommandLine->ClientID%>').select();
			window.scrollTo(0, document.body.scrollHeight);
		}});
	};
	$('console_launcher').observe('click', function(){
		show_hide_console();
	});
</script>
<com:TActivePanel ID="ConsoleContainer" DefaultButton="Enter" Style="text-align: left; display: none;">
	<com:TActiveTextBox ID="OutputListing" TextMode="MultiLine" CssClass="console" ReadOnly="true" />
	<com:TActiveTextBox ID="CommandLine" TextMode="SingleLine" CssClass="textbox" Width="760px" Style="margin: 3px 7px; float: left" />
	<com:TActiveButton ID="Enter" Text="<%[ Enter ]%>" OnCallback="sendCommand">
		<prop:ClientSide.OnLoading>
			$('<%=$this->CommandLine->ClientID%>').disabled = true;
		</prop:ClientSide.OnLoading>
		<prop:ClientSide.OnComplete>
			$('<%=$this->OutputListing->ClientID%>').scrollTop = $('<%=$this->OutputListing->ClientID%>').scrollHeight;
			$('<%=$this->CommandLine->ClientID%>').disabled = false;
			$('<%=$this->CommandLine->ClientID%>').select();
		</prop:ClientSide.OnComplete>
	</com:TActiveButton>
	 <com:TActiveButton ID="Clear" Text="<%[ Clear ]%>" OnCallback="clearConsole" Style="margin: auto 5px; " />
	<script type="text/javascript">
	$('<%=$this->CommandLine->ClientID%>').stopObserving('keydown');
	$('<%=$this->CommandLine->ClientID%>').observe('keydown', function(e) {
		if (e.keyCode == 27) { // close console on pressed ESC key
			show_hide_console();
		}
	});
	</script>
</com:TActivePanel>
