<div id="msg_envelope_modal" class="w3-modal">
	<div class="w3-modal-content w3-animate-top w3-card-4">
		<header class="w3-container w3-green">
			<span onclick="MsgEnvelope.close();" class="w3-button w3-display-topright">&times;</span>
			<h2><%[ Messages ]%></h2>
		</header>
		<div class="w3-container w3-margin-left w3-margin-right">
			<div id="msg_envelope_container" class="w3-code" style="font-size: 12px; min-height: 50px; max-height: 610px; overflow-y: scroll; overflow-x: auto;">
				<pre id="msg_envelope_content"></pre>
			</div>
		</div>
		<footer class="w3-container w3-center">
			<button class="w3-button w3-red w3-section w3-margin-right" onclick="msg_envelope_truncate();"><i class="fas fa-cut"></i> &nbsp;<%[ Truncate log ]%></button>
			<button class="w3-button w3-red w3-section" onclick="MsgEnvelope.close();"><i class="fas fa-times"></i> &nbsp;<%[ Close ]%></button>
		</footer>
	</div>
</div>
<com:TCallback
	ID="MsgEnvelopeTruncate"
	OnCallback="truncate"
	ClientSide.OnComplete="MsgEnvelope.set_logs([]); MsgEnvelope.mark_envelope_ok();"
/>
<script>
function msg_envelope_truncate() {
	var cb = <%=$this->MsgEnvelopeTruncate->ActiveControl->Javascript%>;
	cb.dispatch();
}
MsgEnvelope.init();
</script>
