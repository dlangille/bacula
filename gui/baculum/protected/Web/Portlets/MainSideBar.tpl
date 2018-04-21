<!-- Sidebar/menu -->
<nav class="w3-sidebar w3-white w3-animate-left w3-margin-bottom" style="z-index:3;width:300px;" id="sidebar"><br />
	<div class="w3-container w3-row">
		<div class="w3-col s4">
			<img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/avatar2.png" class="w3-circle w3-margin-right" style="width:46px" />
		</div>
		<div class="w3-col s8 w3-bar">
			<span><%[ Welcome ]%>, <strong><%=$_SERVER['PHP_AUTH_USER']%></strong></span><br>
			<com:TActiveLinkButton
				ID="Logout"
				OnCommand="logout"
				CssClass="w3-bar-item w3-button"
				ToolTip="<%[ Logout ]%>"
			>
				<i class="fa fa-power-off"></i>
			</com:TActiveLinkButton>
			<a href="<%=$this->Service->constructUrl('Console')%>" class="w3-bar-item w3-button" title="<%[ Console ]%>"><i class="fa fa-terminal"></i></a>
			<a href="<%=$this->Service->constructUrl('ApplicationSettings')%>" class="w3-bar-item w3-button<%=!$_SESSION['admin'] ? ' hide' : ''%>" title="<%[ Application settings ]%>"><i class="fa fa-cog"></i></a>
		</div>
	</div>
	<hr />
	<div class="w3-container">
		<h5>Baculum Menu</h5>
	</div>
	<div class="w3-bar-block" style="margin-bottom: 45px;">
		<a href="#" class="w3-bar-item w3-button w3-padding-16 w3-black w3-hover-black" onclick="W3SideBar.close(); return false;" title="close menu">  Close Menu <i class="fa fa-window-close fa-fw w3-right w3-xlarge"></i></a>
		<a href="<%=$this->Service->constructUrl('Dashboard')%>" class="w3-bar-item w3-button w3-padding<%=$this->Service->getRequestedPagePath() == 'Dashboard' ? ' w3-blue': ''%>"><i class="fa fa-tachometer-alt fa-fw"></i>  <%[ Dashboard ]%></a>
		<a href="<%=$this->Service->constructUrl('JobHistoryList')%>" class="w3-bar-item w3-button w3-padding<%=in_array($this->Service->getRequestedPagePath(), array('JobHistoryList', 'JobHistoryView')) ? ' w3-blue': ''%>"><i class="fa fa-history fa-fw"></i>  <%[ Job history ]%></a>
		<a href="<%=$this->Service->constructUrl('JobList')%>" class="w3-bar-item w3-button w3-padding<%=in_array($this->Service->getRequestedPagePath(), array('JobList', 'JobView')) ? ' w3-blue': ''%>"><i class="fa fa-tasks fa-fw"></i>  <%[ Jobs ]%></a>
		<a href="<%=$this->Service->constructUrl('ClientList')%>" class="w3-bar-item w3-button w3-padding<%=in_array($this->Service->getRequestedPagePath(), array('ClientList', 'ClientView')) ? ' w3-blue': ''%>"><i class="fa fa-desktop fa-fw"></i>  <%[ Clients ]%></a>
		<a href="<%=$this->Service->constructUrl('StorageList')%>" class="w3-bar-item w3-button w3-padding<%=in_array($this->Service->getRequestedPagePath(), array('StorageList', 'StorageView', 'DeviceView')) ? ' w3-blue': ''%><%=!$_SESSION['admin'] ? ' hide' : ''%>"><i class="fa fa-database fa-fw"></i>  <%[ Storages ]%></a>
		<a href="<%=$this->Service->constructUrl('PoolList')%>" class="w3-bar-item w3-button w3-padding<%=in_array($this->Service->getRequestedPagePath(), array('PoolList', 'PoolView')) ? ' w3-blue': ''%><%=!$_SESSION['admin'] ? ' hide' : ''%>"><i class="fa fa-tape fa-fw"></i>  <%[ Pools ]%></a>
		<a href="<%=$this->Service->constructUrl('VolumeList')%>" class="w3-bar-item w3-button w3-padding<%=in_array($this->Service->getRequestedPagePath(), array('VolumeList', 'VolumeView')) ? ' w3-blue': ''%><%=!$_SESSION['admin'] ? ' hide' : ''%>"><i class="fa fa-hdd fa-fw"></i>  <%[ Volumes ]%></a>
		<a href="<%=$this->Service->constructUrl('FileSetList')%>" class="w3-bar-item w3-button w3-padding<%=in_array($this->Service->getRequestedPagePath(), array('FileSetList', 'FileSetView')) ? ' w3-blue': ''%><%=!$_SESSION['admin'] ? ' hide' : ''%>"><i class="fa fa-copy fa-fw"></i>  <%[ FileSets ]%></a>
		<a href="<%=$this->Service->constructUrl('ScheduleList')%>" class="w3-bar-item w3-button w3-padding<%=in_array($this->Service->getRequestedPagePath(), array('ScheduleList', 'ScheduleView')) ? ' w3-blue': ''%><%=!$_SESSION['admin'] ? ' hide' : ''%>"><i class="fa fa-clock fa-fw"></i>  <%[ Schedules ]%></a>
		<a href="<%=$this->Service->constructUrl('ConfigureHosts')%>" class="w3-bar-item w3-button w3-padding<%=$this->Service->getRequestedPagePath() == 'ConfigureHosts' ? ' w3-blue': ''%><%=!$_SESSION['admin'] ? ' hide' : ''%>"><i class="fa fa-cog fa-fw"></i>  <%[ Configure ]%></a>
		<a href="<%=$this->Service->constructUrl('RestoreWizard')%>" class="w3-bar-item w3-button w3-padding<%=$this->Service->getRequestedPagePath() == 'RestoreWizard' ? ' w3-blue': ''%>"><i class="fa fa-reply fa-fw"></i>  <%[ Restore wizard ]%></a>
		<a href="<%=$this->Service->constructUrl('Graphs')%>" class="w3-bar-item w3-button w3-padding<%=$this->Service->getRequestedPagePath() == 'Graphs' ? ' w3-blue': ''%>"><i class="fa fa-chart-pie fa-fw"></i>  <%[ Graphs ]%></a>
		<a href="<%=$this->Service->constructUrl('WebConfigWizard')%>" class="w3-bar-item w3-button w3-padding<%=$this->Service->getRequestedPagePath() == 'WebConfigWizard' ? ' w3-blue': ''%><%=!$_SESSION['admin'] ? ' hide' : ''%>"><i class="fa fa-wrench fa-fw"></i>  <%[ Settings ]%></a>
		<a href="<%=$this->Service->constructUrl('Users')%>" class="w3-bar-item w3-button w3-padding<%=$this->Service->getRequestedPagePath() == 'Users' ? ' w3-blue': ''%><%=!$_SESSION['admin'] ? ' hide' : ''%>"><i class="fa fa-users fa-fw"></i>  <%[ Users ]%></a>
	</div>
</nav>

<!-- Overlay effect when opening sidebar on small screens -->
<div class="w3-overlay w3-hide-large w3-animate-opacity" onclick="W3SideBar.close(); return false;" style="cursor:pointer" title="close side menu" id="overlay_bg"></div>
